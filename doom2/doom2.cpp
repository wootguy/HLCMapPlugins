#include "extdll.h"
#include "util.h"
#include "weapons.h"
#include "PluginHooks.h"
#include "CBasePlayer.h"
#include "Scheduler.h"
#include "shake.h"
#include "te_effects.h"

#include "doom2.h"
#include "doom_utils.h"
#include "CDoomMonster.h"

using namespace std;

// TODO:
// delay moving platforms

// TODO (bugs I'm ignoring cuz 2 lazy):
// oriented fireballs aren't always visible (seems related to amount of active monsters)
// pain elemental gets stuck when shooting shulls sometimes 
// revived monsters sometimes invisible until your view angle changes
// map11 souls trying to kill each other but hitting ceiling
// monsters aim too high
// somehow exceeding ammo limits (dropped weapons?)
// weapon sprites skip frames with high ping (need to redo everything with models :'<)
// fall through level in dead simple next to teleport at start
// cacdemon gets stuck at tiny lips when it could easily float around them
// player models should be doom guy sprite (colored?)
// doors that monsters can open
// monsters should open doors, react to sounds without sight
// health/armor/ammo hud?
// probably need to limit sound graph per level
// lite textures need editing (full bar)
// You got the "X"! messages
// items don't get correct brightness
// items sometimes sink into ground && u cant pickup
// solid fireballs bounce off each other (tried SOLID_TOUCH already, projectiles kinda have to be solid)
// crushers should go past monsters || go up after a while
// use MOVETYPE_FOLLOW to reduce net usage (tried it but monsters flicker because sprite stops following when nodraw applied)
// (doom door breaks regular doors): 10:11 AM - Streamfaux: Yeah better be waiting. Also you should investigate this just in case. Putting a door with a targetname && a button targgeting it should be enough to test. And I meanfunc_door && func_button.
// being revived breaks weapons with mp_weapon_droprules 1
// teleport on exit logic (tricks && traps imp room)

// NOTE: ep2 needs Normalized clip type || else you fall through level in tricks && traps near end-tele
// NOTE: Compile options = clip economy + cliptype normalized + 25 min light + RAD no load textures + tex reflect 1.3 + 3 bounces

float g_level_time = 0;
int g_secrets = 0;
int g_kills = 0;
int g_item_gets = 0;
int g_total_secrets = 0;
int g_total_monsters = 0;
int g_total_items = 0;
int g_keys = 0;
int g_map_num = 1;
bool g_strict_keys = false; // if false, only color of key matters when opening door

const int UNLOCK_CHAINSAW = 1;
const int UNLOCK_SHOTGUN = 2;
const int UNLOCK_SUPER_SHOTGUN = 4;
const int UNLOCK_CHAINGUN = 8;
const int UNLOCK_RPG = 16;
const int UNLOCK_PLASMA = 32;
const int UNLOCK_BFG = 64;
const int UNLOCK_PERFECT_REWARD = 128;

bool loadedUnlocks = false;
int g_rewards = 0;
int g_unlocks = 0;

bool g_wait_for_noobs = true;
bool g_friendly_fire = false;
bool g_timer_started = false;
float g_unblock_time = 0;
float g_noob_delay = 20; // timer value when new player joins who hasn't accepted the bug disclaimer

bool g_game_over = false; // final map complete
bool debug_mode = false;

cvar_t* g_dmgScale;

vector<int> g_par_times = { 30, 90, 120, 120, 90, 150, 120, 120, 270, 90, 210 };
vector<const char*> g_map_music = {
	"doom/running_from_evil.mp3",
	"doom/the_healer_stalks.mp3",
	"doom/countdown_to_death.mp3",
	"doom/between_levels.mp3",
	"doom/doom.mp3",
	"doom/in_the_dark.mp3",
	"doom/shawn_shotgun.mp3",
	"doom/taylor_blues.mp3",
	"doom/sandy_city.mp3",
	"doom/the_demons_dead.mp3",
	"doom/the_healer_stalks.mp3",
};
const char* g_inter_music = "doom/intermission.mp3";
const char* g_ep_music = "doom/episode.mp3";

Vector g_spawn_room_pos;

// list of ents that need brightness updated manually
vector<EHANDLE> g_illuminate_ents;

class VisEnt
{
	bool visible;
	EHANDLE sprite;

	VisEnt() {}

	VisEnt(bool visible, EHANDLE sprite)
	{
		this->visible = visible;
		this->sprite = sprite;
	}
};

unordered_map<uint64_t, PlayerState> g_player_states;

vector<string> sprite_angles = {
	"1", "2?8", "3?7", "4?6", "5", "6?4", "7?3", "8?2"
};

int g_blud_sprite;

string base36(int num)
{
	string b36;
	const char* charset = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	while (num != 0)
	{
		char c = charset[num % 36];
		b36 = c + b36;
		num /= 36;
	}
	return b36;
}

HOOK_RETURN_DATA MapInit()
{
	g_wait_for_noobs = strstr(STRING(gpGlobals->mapname), "doom2_ep1") == 0;
	/*
	PrecacheModel("models/hlclassic/p_9mmhandgun.mdl");
	PrecacheModel("models/hlclassic/p_egon.mdl");
	PrecacheModel("models/hlclassic/p_gauss.mdl");
	PrecacheModel("models/hlclassic/p_rpg.mdl");
	PrecacheModel("models/hlclassic/p_shotgun.mdl");
	PrecacheModel("models/hlclassic/p_shotgun.mdl");
	PrecacheModel("models/custom_weapons/cs16/p_chainsaw.mdl");
	PrecacheModel("models/custom_weapons/cs16/p_m1887.mdl");

	PrecacheSound("doom/dsskldth.wav"); // player use
	PrecacheSound("doom/dsplpain.wav"); // player pain
	PrecacheSound("doom/dspldeth.wav"); // player death
	PrecacheSound("doom/dsitmbk.wav"); // item respawn
	PrecacheSound("doom/dsitemup.wav"); // item collect
	PrecacheSound("doom/dssecret.wav"); // secret revealed
	*/

	for (int i = 0; i < g_map_music.size(); i++)
		PRECACHE_SOUND_NULLENT(g_map_music[i]);

	PRECACHE_SOUND_NULLENT(g_inter_music);
	PRECACHE_SOUND_NULLENT(g_ep_music);
	PRECACHE_SOUND_NULLENT("doom/dsplpain.wav");
	PRECACHE_SOUND_NULLENT("doom/dspldeth.wav");
	PRECACHE_SOUND_NULLENT("doom/dsfirsht.wav");

	g_blud_sprite = PRECACHE_MODEL_NULLENT("sprites/doom/blud.spr");

	return HOOK_CONTINUE;
}

HOOK_RETURN_DATA MapActivate()
{
	/*
	vector<CBaseEntity*> doors;
	vector<CBaseEntity*> buttons;

	CBaseEntity* ent = NULL;
	do {
		ent = UTIL_FindEntityByClassname(ent, "func_doom_door");
		if (ent)
		{
			func_doom_door* door = cast<func_doom_door*>(CastToScriptClass(ent));
			if (door.isButton)
				buttons.push_back(ent);
			else
				doors.push_back(ent);
		}
	} while (ent);

	*ent = NULL;
	do {
		*ent = UTIL_FindEntityByClassname(ent, "env_sprite");
		if (ent)
		{
			SET_MODEL(ent, fixPath("sprites/doom/text.spr"));
		}
	} while (ent);

	for (uint i = 0; i < buttons.length(); i++)
	{
		for (uint k = 0; k < doors.length(); k++)
		{
			if (buttons[i]->pev->spawnflags & FL_DOOR_BUTTON_DONT_MOVE == 0 && buttons[i].Intersects(doors[k]))
			{
				//println("GOT INTERSECT " + buttons[i]->pev->targetname + " " + doors[k]->pev->targetname);
				func_doom_door* button = cast<func_doom_door*>(CastToScriptClass(buttons[i]));
				func_doom_door* door = cast<func_doom_door*>(CastToScriptClass(doors[k]));
				button.dir = door.dir;
				button.m_flLip = door.m_flLip;
				button->pev->speed = door->pev->speed;
				button.m_vecPosition2 = button->pev->origin + Vector(0, 0, (button.dir * (door->pev->size.z - 2)) - button.dir * button.m_flLip);

				door.sync_buttons.push_back(EHANDLE(buttons[i]));
				button.parent = ent;
			}
		}
	}

	CBaseEntity* map_info = UTIL_FindEntityByTargetname(NULL, "map_info");
	if (map_info)
	{
		g_map_num += (map_info->pev->renderfx - 1);
		//println("INITIAL MAP: " + g_map_num);
	}

	CBaseEntity* spawn_room = UTIL_FindEntityByTargetname(NULL, "map_start");
	g_spawn_room_pos = spawn_room ? spawn_room->pev->origin : Vector(0, 0, 0);

	dictionary keys;
	keys["origin"] = g_spawn_room_pos.ToString();
	keys["targetname"] = "secret_revealed";
	keys["m_iszScriptFile"] = "doom/doom.as";
	keys["m_iszScriptFunctionName"] = "secret_revealed";
	keys["m_iMode"] = "1";
	keys["delay"] = "0";
	CreateEntity("trigger_script", keys, true);


	keys["targetname"] = "inter_music";
	keys["volume"] = "10";
	keys["message"] = g_inter_music;
	keys["spawnflags"] = "3";
	CreateEntity("ambient_music", keys, true);

	keys["targetname"] = "ep_music";
	keys["volume"] = "10";
	keys["message"] = g_ep_music;
	keys["spawnflags"] = "3";
	CreateEntity("ambient_music", keys, true);

	createSoundGraph();
	*/

	return HOOK_CONTINUE;
}

void clearTimer()
{
	ALERT(at_error, "TODO: timer\n");
	/*
	HUDNumDisplayParams params;
	params.channel = 15;
	params.flags = HUD_ELEM_HIDDEN;
	g_PlayerFuncs.HudTimeDisplay(NULL, params);
	*/
}

// thanks th_escape for le codes :>
void updateTimer()
{
	ALERT(at_error, "TODO: timer\n");
	/*
	HUDNumDisplayParams params;

	params.channel = 15;

	params.flags = HUD_ELEM_SCR_CENTER_X | HUD_ELEM_DEFAULT_ALPHA |
		HUD_TIME_MINUTES | HUD_TIME_SECONDS | HUD_TIME_COUNT_DOWN;
		*/

	float timeLeft = g_unblock_time - gpGlobals->time;
	/*
	params.value = timeLeft;

	params.x = 0;
	params.y = 0.06;
	params.color1 = RGBA_SVENCOOP;
	params.spritename = "stopwatch";

	vector<CBaseEntity*> waitingPlayers;

	CBaseEntity* ent = NULL;
	do {
		*ent = UTIL_FindEntityByClassname(ent, "player");
		if (ent) {
			CBasePlayer* plr = (CBasePlayer*)(ent);
			waitingPlayers.push_back(ent);
		}
	} while (ent);

	for (uint i = 0; i < waitingPlayers.length(); i++)
	{
		g_PlayerFuncs.HudTimeDisplay(NULL, params);
	}
	*/

	if (timeLeft > 0) {
		g_Scheduler.SetTimeout(updateTimer, 1.0f);
	}
	else {
		clearTimer();
		FireTargets("ep_wall", NULL, NULL, USE_ON);
		g_wait_for_noobs = false;
		return;
	}

}

void resetTimer()
{
	g_timer_started = true;
	g_unblock_time = gpGlobals->time + g_noob_delay;
	updateTimer();
}

HOOK_RETURN_DATA PlayerPostThink(CBasePlayer* plr)
{
	/*
	PlayerState* state = getPlayerState(plr);

	HUDSpriteParams params;
	string hud_sprite = fixPath("sprites/doom/keys.spr");
	params.spritename = hud_sprite.SubString("sprites/".Length()); // so resguy doesn't get confused
	params.width = 0;
	params.flags = HUD_SPR_MASKED | HUD_ELEM_ABSOLUTE_Y | HUD_ELEM_ABSOLUTE_X;
	params.holdTime = 99999.0f;
	params.color1 = RGBA(255, 255, 255, 255);

	float sprScale = g_spr_scales[state.uiScale];
	int sprHeight = int(sprScale * 6);
	float baseX = -sprScale * 5 * 2;
	float baseY = 50;
	params.x = baseX;
	params.y = baseY;

	if (state.lastHudKeys + 10.0f < gpGlobals->time || state.hudKeys != g_keys)
	{
		state.lastHudKeys = gpGlobals->time;
		state.hudKeys = g_keys;
		for (uint i = 0; i < 6; i++)
		{
			params.channel = 9 + i;
			params.frame = state.uiScale * 6 + i;
			if (i == 3) // skull keys
			{
				params.y = baseY - sprScale * 2;
				params.x = baseX + sprScale * 9;
			}

			if (g_keys & (1 << i) == 0)
			{
				HUDSpriteParams offparams;
				offparams.channel = params.channel;
				g_PlayerFuncs.HudCustomSprite(plr, offparams);
				continue;
			}

			g_PlayerFuncs.HudCustomSprite(plr, params);

			params.y += sprScale * 2 + sprHeight;
		}
	}

	//g_SoundSystem.StopSound(ent->edict(), CHAN_BODY, "player/pl_step3.wav");
	//g_SoundSystem.StopSound(ent->edict(), CHAN_BODY, "player/pl_step6.wav");

	//g_PlayerFuncs.HudToggleElement(plr, tile, false);

	//ent->pev->view_ofs.z = 20; // original == 28
	//ent->pev->scale = 0.7f;
	//ent->pev->fuser4 = 2;
	//println("HEIGHT: " + (ent->pev->origin.z + ent->pev->view_ofs.z) + " " + ent->pev->view_ofs.z);
	return HOOK_CONTINUE;
	*/

	return HOOK_CONTINUE;
}

void player_killed(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
{
	CBasePlayer* plr = pCaller->MyPlayerPointer();
	if (!plr)
		return;
	PlayerState& state = getPlayerState(plr);
	state.lastSuit = state.lastGoggles = state.lastGod = state.lastInvis = 0;
}

void secret_revealed(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
{
	EMIT_SOUND_DYN(pActivator->edict(), CHAN_STATIC, "doom/dssecret.wav", 1.0f, ATTN_NONE, 0, 100);
	UTIL_ClientPrintAll(print_center, "A SECRET IS REVEALED!\n");
	g_secrets += 1;
}

void printkeybind(EHANDLE h_plr, string msg)
{
	if (!h_plr)
		return;
	CBasePlayer* plr = (CBasePlayer*)(h_plr.GetEntity());

	UTIL_ClientPrint(plr, print_center, msg.c_str());
}

string getMapName()
{
	return UTIL_VarArgs("map%02d", g_map_num);
}

void level_started(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
{
	g_level_time = gpGlobals->time;

	g_secrets = 0;
	g_kills = 0;
	g_item_gets = 0;
	g_total_secrets = 0;
	g_total_monsters = 0;
	g_total_items = 0;

	string minsName = getMapName() + "_mins";
	CBaseEntity* minsEnt = UTIL_FindEntityByTargetname(NULL, (getMapName() + "_mins").c_str());
	CBaseEntity* maxsEnt = UTIL_FindEntityByTargetname(NULL, (getMapName() + "_maxs").c_str());

	if (!minsEnt || !maxsEnt) {
		ALERT(at_error, "Missing level mins/maxs. Level start failed.");
		return;
	}

	Vector level_min = minsEnt->pev->origin;
	Vector level_max = maxsEnt->pev->origin;

	CBaseEntity* ent = NULL;
	do {
		ent = UTIL_FindEntityByClassname(ent, "*");
		if (ent)
		{
			if (ent->pev->absmin.x > level_min.x && ent->pev->absmin.y > level_min.y && ent->pev->absmin.z > level_min.z &&
				ent->pev->absmax.x < level_max.x && ent->pev->absmax.y < level_max.y && ent->pev->absmax.z < level_max.z)
			{
				if (string(STRING(ent->pev->targetname)).find("strobe") == 0)
					continue; // HACK: fixes strobe arrows in tunnels

				// add prefix to entity names so multiple levels don't conflict
				string prefix = getMapName() + "_";
				string cname = string(STRING(ent->pev->classname));
				string targ = string(STRING(ent->pev->target));
				if (string(STRING(ent->pev->targetname)).length() > 0)
					ent->pev->targetname = ALLOC_STRING((prefix + STRING(ent->pev->targetname)).c_str());
				if (targ.length() > 0 && targ != "secret_revealed" && targ != "teleport_thing" && targ != "exit_level")
					ent->pev->target = ALLOC_STRING((prefix + targ).c_str());
				if (targ == "teleport_thing")
					ent->pev->netname = ALLOC_STRING((prefix + STRING(ent->pev->netname)).c_str());

				if (cname == "trigger_changevalue")
					ent->pev->message = ALLOC_STRING((prefix + STRING(ent->pev->message)).c_str());

				if (cname == "trigger_once" && targ == "secret_revealed")
					g_total_secrets += 1;

				if (cname.find("monster_") == 0)
				{
					CBaseMonster* bmon = ent->MyMonsterPointer();
					CDoomMonster* mon = (CDoomMonster*)bmon;
					if (bmon)
						bmon->m_iszTriggerTarget = ALLOC_STRING((prefix + STRING(bmon->m_iszTriggerTarget)).c_str());
					if (mon)
						mon->Setup();
					g_total_monsters += 1;
				}

				ALERT(at_error, "TODO: Item init\n");
				/*
				if (string(STRING(ent->pev->classname)).find("item_doom_") == 0)
				{
					item_doom* item = cast<item_doom*>(CastToScriptClass(ent));
					if (item.intermission)
						g_total_items += 1;
				}
				*/
			}
		}
	} while (ent);


	StringMap keys = {
		{"targetname", (getMapName() + "_music").c_str()},
		{"volume", "10" },
		{"message", g_map_music[g_map_num - 1] },
		{"spawnflags", "3" }
	};
	CBaseEntity::Create("ambient_music", g_spawn_room_pos, g_vecZero, true, NULL, keys);

	FireTargets((getMapName() + "_spawns").c_str(), NULL, NULL, USE_ON);
	FireTargets((getMapName() + "_music").c_str(), NULL, NULL, USE_ON);
}

void episode_end()
{
	CBaseEntity* trans = UTIL_FindEntityByTargetname(NULL, "inter_fin_spr");
	CBaseEntity* lvl = UTIL_FindEntityByTargetname(NULL, "inter_lvl_spr");
	trans->pev->effects |= EF_NODRAW;
	lvl->pev->effects |= EF_NODRAW;
	FireTargets("ep_end", NULL, NULL, USE_TOGGLE);
}

void trigger_next_level()
{
	FireTargets("map_start", NULL, NULL, USE_TOGGLE);
}

void next_level()
{
	vector<string> sprItems = { "kills", "items", "secret", "time", "par" };
	for (int i = 0; i < sprItems.size(); i++)
	{
		CBaseEntity* label = UTIL_FindEntityByTargetname(NULL, ("inter_" + sprItems[i] + "_spr").c_str());
		if (label)
			label->pev->effects |= EF_NODRAW;
		for (int k = 0; k < 5; k++)
		{
			CBaseEntity* spr = UTIL_FindEntityByTargetname(NULL, UTIL_VarArgs("inter_%s_spr%d", sprItems[i].c_str(), k));
			if (spr)
				spr->pev->effects |= EF_NODRAW;
		}
	}

	CBaseEntity* trans = UTIL_FindEntityByTargetname(NULL, "inter_fin_spr");
	CBaseEntity* lvl = UTIL_FindEntityByTargetname(NULL, "inter_lvl_spr");

	Vector temp = trans->pev->origin;
	trans->pev->origin = lvl->pev->origin;
	lvl->pev->origin = temp;

	trans->pev->frame = 52;
	lvl->pev->frame += 1;

	FireTargets("next_level", NULL, NULL, USE_TOGGLE);

	g_map_num++;

	if (g_map_num == 7)
	{
		//g_Scheduler.SetTimeout("episode_end", 1.0f);
		FireTargets("change_level", NULL, NULL, USE_TOGGLE);
	}
	else
		g_Scheduler.SetTimeout(trigger_next_level, 3.0f);
}

void end_game()
{
	FireTargets("end", NULL, NULL, USE_TOGGLE);
}

void printkeybindall(string msg)
{
	UTIL_ClientPrintAll(print_center, msg.c_str());
}

void end_game_dm()
{
	StringMap ckeys{
		{"targetname", "dm_equip"},
		{"spawnflags", "4"},
		{"weapon_doom_bfg", "1"},
		{"weapon_doom_plasmagun", "1"},
		{"weapon_doom_rpg", "1"},
		{"weapon_doom_chainsaw", "1"},
		{"weapon_doom_supershot", "1"},
		{"ammo_doom_shellbox", "1"},
		{"ammo_doom_rocketbox", "5"},
		{"ammo_doom_cells", "1"},
	};

	g_friendly_fire = true;

	CBaseEntity* equip = CBaseEntity::Create("game_player_equip", g_vecZero, g_vecZero, true, NULL, ckeys);

	CBaseEntity* ent = NULL;
	do {
		ent = UTIL_FindEntityByClassname(ent, "player");
		if (ent && ent->IsAlive())
		{
			FireTargets("dm_equip", ent, ent, USE_TOGGLE);
		}
	} while (ent);
}

void loner_check()
{
	int numPlayers = 0;

	CBaseEntity* ent = NULL;
	do {
		ent = UTIL_FindEntityByClassname(ent, "player");
		if (ent) {
			numPlayers++;
		}
	} while (ent);

	if (numPlayers == 1) {
		string msg = "Oh, you're alone. How sad.";
		g_Scheduler.SetTimeout(printkeybindall, 0.0f, msg);
		g_Scheduler.SetTimeout(printkeybindall, 1.0f, msg);
	}
}

void unlock_weapon(bool notify)
{

}

void unlock_item(bool notify, int type)
{
	if (g_rewards >= 10 || type == 0)
		return;

	bool isReward = type == UNLOCK_PERFECT_REWARD;
	if (isReward)
		g_rewards++;
	else
	{
		if (g_unlocks & type != 0)
			return;
		g_unlocks |= type;
	}

	StringMap ckeys;
	string msg = isReward ? "PERFECT SCORE\nReward: " : "WEAPON DISCOVERED\n";

	if (isReward)
	{
		switch (g_rewards)
		{
		case 1:
			msg += "Extra Ammo";
			ckeys.put("ammo_doom_shells", "2");
			ckeys.put("ammo_doom_bullets", "5");
			break;
		case 2:
			msg += "Extra ammo";
			ckeys.put("ammo_doom_bulletbox", "1");
			ckeys.put("ammo_doom_shellbox", "1");
			ckeys.put("ammo_doom_rocket", "1");
			break;
		case 3:
			msg += "Armor";
			ckeys.put("item_doom_armor", "1");
			break;
		case 4:
			msg += "Extra ammo";
			ckeys.put("ammo_doom_bulletbox", "1");
			ckeys.put("ammo_doom_shellbox", "1");
			ckeys.put("ammo_doom_rocket", "2");
			break;
		case 5:
			msg += "Extra ammo";
			ckeys.put("ammo_doom_shellbox", "1");
			ckeys.put("ammo_doom_cells", "3");
			ckeys.put("ammo_doom_rocketbox", "1");
			break;
		case 6:
			msg += "Mega Armor";
			ckeys.put("item_doom_megaarmor", "1");
			break;
		case 7:
			msg += "Extra ammo";
			ckeys.put("ammo_doom_cellbox", "1");
			ckeys.put("ammo_doom_rocketbox", "2");
			break;
		case 8:
			msg += "Extra ammo";
			ckeys.put("ammo_doom_cellbox", "1");
			ckeys.put("ammo_doom_rocketbox", "4");
			break;
		case 9:
			msg += "Full ammo";
			ckeys.put("ammo_doom_cellbox", "3");
			ckeys.put("ammo_doom_rocketbox", "12");
			break;
		case 10:
			msg += "Soul Sphere";
			ckeys.put("item_doom_soulsphere", "1");
			break;
		}
	}
	else
	{
		switch (type)
		{
		case UNLOCK_CHAINSAW:
			msg += "Chainsaw";
			ckeys.put("weapon_doom_chainsaw", "1");
			break;
		case UNLOCK_SHOTGUN:
			msg += "Shotgun";
			ckeys.put("weapon_doom_shotgun", "1");
			break;
		case UNLOCK_CHAINGUN:
			msg += "Chaingun";
			ckeys.put("weapon_doom_chaingun", "1");
			break;
		case UNLOCK_SUPER_SHOTGUN:
			msg += "Super Shotgun";
			ckeys.put("weapon_doom_supershot", "1");
			break;
		case UNLOCK_RPG:
			msg += "Rocket Launcher";
			ckeys.put("weapon_doom_rpg", "1");
			break;
		case UNLOCK_PLASMA:
			msg += "Plasma Gun";
			ckeys.put("weapon_doom_plasmagun", "1");
			break;
		case UNLOCK_BFG:
			msg += "BFG";
			ckeys.put("weapon_doom_bfg", "1");
			break;
		}
	}

	Vector cori = UTIL_FindEntityByTargetname(NULL, "unlock_counter")->pev->origin;
	ckeys.put("spawnflags", "8");
	CBaseEntity* equip = CBaseEntity::Create("game_player_equip", cori, g_vecZero, true, NULL, ckeys);

	ckeys.put("spawnflags", "1");
	if (isReward)
		ckeys.put("targetname", "use_reward" + g_rewards);
	else
			ckeys.put("targetname", "use_unlock" + type);
	CBaseEntity* equipuse = CBaseEntity::Create("game_player_equip", cori, g_vecZero, true, NULL, ckeys);

	if (!notify) {
		return;
	}

	CBaseEntity* ent = NULL;
	do {
		ent = UTIL_FindEntityByClassname(ent, "player");
		if (ent && ent->IsAlive()) {
			CBasePlayer* plr = (CBasePlayer*)(ent);
			FireTargets(STRING(equipuse->pev->targetname), ent, ent, USE_ON);
		}
	} while (ent);

	CBaseEntity* count = UTIL_FindEntityByTargetname(NULL, "unlock_counter");
	count->pev->frags = g_rewards;
	count->pev->health = g_unlocks;

	EMIT_SOUND_DYN(NULL, CHAN_STATIC, "doom/dssecret.wav", 1.0f, ATTN_NONE, 0, 100);
	g_Scheduler.SetTimeout(printkeybindall, 0.0f, msg);
	g_Scheduler.SetTimeout(printkeybindall, 1.0f, msg);

	if (g_rewards >= 10)
		g_Scheduler.SetTimeout(printkeybindall, 5.0f, "All rewards have been given!");
}

void tally_time(string item, int time, int targetTime, bool playSound)
{
	if (time > targetTime)
		time = targetTime;

	CBaseEntity* minTens = UTIL_FindEntityByTargetname(NULL, ("inter_" + item + "_spr0").c_str());
	CBaseEntity* minOnes = UTIL_FindEntityByTargetname(NULL, ("inter_" + item + "_spr1").c_str());
	CBaseEntity* colon = UTIL_FindEntityByTargetname(NULL, ("inter_" + item + "_spr2").c_str());
	CBaseEntity* secTens = UTIL_FindEntityByTargetname(NULL, ("inter_" + item + "_spr3").c_str());
	CBaseEntity* secOnes = UTIL_FindEntityByTargetname(NULL, ("inter_" + item + "_spr4").c_str());

	int numFrameStart = 57;

	colon->pev->effects &= ~EF_NODRAW;
	secOnes->pev->effects &= ~EF_NODRAW;
	secTens->pev->effects &= ~EF_NODRAW;
	minOnes->pev->effects &= ~EF_NODRAW;

	if (time >= 60 * 10)
		minTens->pev->effects &= ~EF_NODRAW;

	// don't loop around
	int showTime = time;
	if (showTime > 60 * 99 + 59)
		showTime = 60 * 99 + 59;

	colon->pev->frame = 51;
	secOnes->pev->frame = numFrameStart + ((showTime % 60) % 10);
	secTens->pev->frame = numFrameStart + (((showTime % 60) / 10) % 10);
	minOnes->pev->frame = numFrameStart + ((showTime / 60) % 10);
	minTens->pev->frame = numFrameStart + (((showTime / 60) / 10) % 10);

	if (time < targetTime)
	{
		if (playSound)
			EMIT_SOUND_DYN(colon->edict(), CHAN_STATIC, "doom/dspistol.wav", 1.0f, 1.0f, 0, 100);
		int step = V_max(targetTime / 15, 3);
		g_Scheduler.SetTimeout(tally_time, 0.05, item, time + step, targetTime, !playSound);
	}
	else
	{
		EMIT_SOUND_DYN(secOnes->edict(), CHAN_STATIC, "doom/dsbarexp.wav", 1.0f, 1.0f, 0, 100);
		if (item == "time")
			g_Scheduler.SetTimeout(tally_time, 0.8, "par", 0, g_par_times[g_map_num - 1], !playSound);
		if (item == "par")
		{
			if (g_map_num == 11) {
				g_Scheduler.SetTimeout(end_game, 22.0);
				CBasePlayer* plr = getAnyPlayer();

				string msg = "ERROR: Mapper too lazy to finish series.\n\nGame ends in 20 seconds.";
				g_Scheduler.SetTimeout(printkeybindall, 2.0f, msg);
				g_Scheduler.SetTimeout(printkeybindall, 3.0f, msg);
				g_Scheduler.SetTimeout(printkeybindall, 4.0f, msg);

				if (!g_friendly_fire)
				{
					msg = "also friendly fire is on now";
					g_Scheduler.SetTimeout(printkeybindall, 6.0f, msg);

					g_Scheduler.SetTimeout(loner_check, 10.0f);
				}

				g_Scheduler.SetTimeout(end_game_dm, 9.0f);

				return;
			}

			bool perfectScore = g_item_gets == g_total_items && g_secrets == g_total_secrets && g_kills == g_total_monsters;

			if (perfectScore) {
				g_Scheduler.SetTimeout(unlock_item, 1.0f, true, UNLOCK_PERFECT_REWARD);
			}
			g_Scheduler.SetTimeout(next_level, perfectScore ? 6.0 : 4.0);
		}
	}
}

void tally_score(string item, int percentage, int targetPercent, bool playSound)
{
	if (percentage > targetPercent)
		percentage = targetPercent;

	CBaseEntity* hundreds = UTIL_FindEntityByTargetname(NULL, ("inter_" + item + "_spr0").c_str());
	CBaseEntity* tens = UTIL_FindEntityByTargetname(NULL, ("inter_" + item + "_spr1").c_str());
	CBaseEntity* ones = UTIL_FindEntityByTargetname(NULL, ("inter_" + item + "_spr2").c_str());
	CBaseEntity* percent = UTIL_FindEntityByTargetname(NULL, ("inter_" + item + "_spr3").c_str());

	int numFrameStart = 40;

	percent->pev->effects &= ~EF_NODRAW;

	ones->pev->effects &= ~EF_NODRAW;
	if (percentage >= 10)
		tens->pev->effects &= ~EF_NODRAW;
	if (percentage >= 100)
		hundreds->pev->effects &= ~EF_NODRAW;

	percent->pev->frame = 50;
	hundreds->pev->frame = numFrameStart + ((percentage / 100) % 10);
	tens->pev->frame = numFrameStart + ((percentage / 10) % 10);
	ones->pev->frame = numFrameStart + (percentage % 10);

	if (percentage < targetPercent)
	{
		if (playSound)
			EMIT_SOUND_DYN(ones->edict(), CHAN_STATIC, "doom/dspistol.wav", 1.0f, 1.0f, 0, 100);
		g_Scheduler.SetTimeout(tally_score, 0.05, item, percentage + 13, targetPercent, !playSound);
	}
	else
	{
		EMIT_SOUND_DYN(tens->edict(), CHAN_STATIC, "doom/dsbarexp.wav", 1.0f, 1.0f, 0, 100);

		if (item == "kills")
		{
			int itemPercentage = 100;
			if (g_total_items > 0)
				itemPercentage = int((g_item_gets / float(g_total_items)) * 100);
			g_Scheduler.SetTimeout(tally_score, 0.8f, "items", 0, itemPercentage, true);
		}
		if (item == "items")
		{
			int secretPercent = 100;
			if (g_total_secrets > 0)
				secretPercent = int((g_secrets / float(g_total_secrets)) * 100);
			g_Scheduler.SetTimeout(tally_score, 0.8f, "secret", 0, secretPercent, true);
		}
		if (item == "secret")
			g_Scheduler.SetTimeout(tally_time, 0.8, "time", 0, int(g_level_time), !playSound);
	}
}

void cleanup_level()
{
	Vector level_min = UTIL_FindEntityByTargetname(NULL, (getMapName() + "_mins").c_str())->pev->origin;
	Vector level_max = UTIL_FindEntityByTargetname(NULL, (getMapName() + "_maxs").c_str())->pev->origin;

	CBaseEntity* ent = NULL;
	do {
		ent = UTIL_FindEntityByClassname(ent, "*");
		if (ent)
		{
			if (ent->pev->absmin.x > level_min.x && ent->pev->absmin.y > level_min.y && ent->pev->absmin.z > level_min.z &&
				ent->pev->absmax.x < level_max.x && ent->pev->absmax.y < level_max.y && ent->pev->absmax.z < level_max.z)
			{
				UTIL_Remove(ent);
			}
		}
	} while (ent);

	g_keys = 0;
}

void intermission(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
{
	FireTargets((getMapName() + "_spawns").c_str(), NULL, NULL, USE_OFF);
	FireTargets((getMapName() + "_music").c_str(), NULL, NULL, USE_OFF);

	g_Scheduler.SetTimeout(cleanup_level, 1.0f);

	CBaseEntity* trans = UTIL_FindEntityByTargetname(NULL, "inter_fin_spr");
	CBaseEntity* lvl = UTIL_FindEntityByTargetname(NULL, "inter_lvl_spr");
	trans->pev->effects &= ~EF_NODRAW;
	lvl->pev->effects &= ~EF_NODRAW;

	Vector temp = trans->pev->origin;
	trans->pev->origin = lvl->pev->origin;
	lvl->pev->origin = temp;
	lvl->pev->frame = g_map_num - 1;
	trans->pev->frame = 53;

	vector<string> sprItems = { "kills", "items", "secret", "time", "par" };
	for (int i = 0; i < sprItems.size(); i++)
	{
		CBaseEntity* label = UTIL_FindEntityByTargetname(NULL, ("inter_" + sprItems[i] + "_spr").c_str());
		if (label)
			label->pev->effects &= ~EF_NODRAW;
	}

	g_level_time = gpGlobals->time - g_level_time;

	int killPercent = 100;
	if (g_total_monsters > 0)
		killPercent = int((g_kills / float(g_total_monsters)) * 100);

	g_Scheduler.SetTimeout(tally_score, 1.5f, "kills", 0, killPercent, true);
	//tally_time("time", 0, 60*8 + 26, true);
}

void ep_scroll_line(int lineNum, int stepsTaken, int maxStep)
{
	CBaseEntity* scroll = UTIL_FindEntityByTargetname(NULL, "ep_scroll" + lineNum);
	if (scroll == NULL)
		return;
	scroll->pev->origin.x += 0.1f;

	if (stepsTaken < maxStep)
	{
		g_Scheduler.SetTimeout(ep_scroll_line, 0.05f, lineNum, stepsTaken + 1, 50);
	}
	else
	{
		g_Scheduler.SetTimeout(ep_scroll_line, 0, lineNum + 1, 0, 50);
	}
}

void ep_text(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
{
	g_Scheduler.SetTimeout(ep_scroll_line, 0.0f, 1, 0, 10);
}

void delay_init_player(EHANDLE h_plr) {
	CBasePlayer* plr = (CBasePlayer*)(h_plr.GetEntity());

	for (int i = 1; i <= g_rewards; i++) {
		FireTargets("use_reward" + i, plr, plr, USE_TOGGLE);
	}
	for (int i = 1; i <= g_unlocks; i <<= 1) {
		FireTargets("use_unlock" + i, plr, plr, USE_TOGGLE);
	}
}

HOOK_RETURN_DATA ClientJoin(CBasePlayer* plr)
{
	PlayerState& state = getPlayerState(plr);

	if (g_wait_for_noobs)
	{
		if (g_timer_started) {
			resetTimer();
		}
	}

	if (!loadedUnlocks) {
		loadedUnlocks = true;

		CBaseEntity* count = UTIL_FindEntityByTargetname(NULL, "unlock_counter");
		if (count) {
			//count->pev->frags = 9;
			//count->pev->health = 0xffff;			
			ALERT(at_console, "Loaded %f frags, %f health\n", count->pev->frags, count->pev->health);
			for (int i = 0; i < int(count->pev->frags); i++) {
				unlock_item(false, UNLOCK_PERFECT_REWARD);
			}
			for (int i = 1; i <= int(count->pev->health) && i < UNLOCK_PERFECT_REWARD; i <<= 1) {
				unlock_item(false, i);
			}
		}
	}

	g_Scheduler.SetTimeout(delay_init_player, 2.0f, EHANDLE(plr->edict()));

	return HOOK_CONTINUE;
}

void new_lobby_player(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
{
	if (g_wait_for_noobs && !g_timer_started) {
		resetTimer();
	}
}

HOOK_RETURN_DATA PlayerUse(CBasePlayer* plr)
{
	if (plr->m_afButtonPressed & IN_USE) {
		TraceResult tr = TraceLook(plr, 90);
		CBaseEntity* phit = CBaseEntity::Instance(tr.pHit);
		if (string(STRING(phit->pev->classname)) == "func_doom_door")
			phit->Use(plr, plr, USE_TOGGLE);
	}

	return HOOK_CONTINUE;
}

bool CmdGiveAll(CBasePlayer* plr, const CommandArgs& args) {
	g_keys = 0xff;
	plr->GiveNamedItem("weapon_doom_fist");
	plr->GiveNamedItem("weapon_doom_chainsaw");
	plr->GiveNamedItem("weapon_doom_pistol");
	plr->GiveNamedItem("weapon_doom_chaingun");
	plr->GiveNamedItem("weapon_doom_shotgun");
	plr->GiveNamedItem("weapon_doom_supershot");
	plr->GiveNamedItem("weapon_doom_rpg");
	plr->GiveNamedItem("weapon_doom_plasmagun");
	plr->GiveNamedItem("weapon_doom_bfg");
	plr->m_rgAmmo[CBasePlayer::GetAmmoIndex("cells")] = 1000000;
	plr->m_rgAmmo[CBasePlayer::GetAmmoIndex("bullets")] = 1000000;
	plr->m_rgAmmo[CBasePlayer::GetAmmoIndex("shells")] = 1000000;
	plr->m_rgAmmo[CBasePlayer::GetAmmoIndex("rockets")] = 1000000;
	return true;
}

bool CmdCheckScore(CBasePlayer* plr, const CommandArgs& args) {
	int killPercentage = 100;
	int itemPercentage = 100;
	int secretPercentage = 100;
	if (g_total_items > 0)
		itemPercentage = int((g_item_gets / float(g_total_items)) * 100);
	if (g_total_monsters > 0)
		killPercentage = int((g_kills / float(g_total_monsters)) * 100);
	if (g_total_secrets > 0)
		secretPercentage = int((g_secrets / float(g_total_secrets)) * 100);
	UTIL_ClientPrint(plr, print_chat, UTIL_VarArgs("Kills: %d%%  Items: %d%%  Secrets: %d%%\n",
		killPercentage, itemPercentage, secretPercentage));
	return true;
}

HOOK_RETURN_DATA PlayerTakeDamage(CBasePlayer* plr, entvars_t* pevInflictor, entvars_t* pevAttacker, float& flDamage, int& bitsDamageType) {
	if (plr->pev->deadflag == DEAD_NO)
	{
		// no pain sound during death animation.
		EMIT_SOUND_DYN(plr->edict(), CHAN_STATIC, "doom/dsplpain.wav", 1.0f, 1.0f, 0, 100);
		UTIL_ScreenFade(plr, Vector(255, 0, 0), 0.2f, 0, 32, FFADE_IN);
	}

	return HOOK_CONTINUE;
}

HOOK_RETURN_DATA DoomBlood(Vector& vecSpot, int& bloodColor, float& flDamage) {
	UTIL_SpriteSpray(vecSpot, Vector(0, 0, 1), g_blud_sprite, 1, 10, 0);

	//int spr1 = MODEL_INDEX("sprites/doom/blud.spr");
	//int spr2 = MODEL_INDEX("sprites/blood.spr");
	//UTIL_BloodSprite(vecOrigin, spr1, spr2, 70, 50);

	return HOOK_CONTINUE_OVERRIDE(0);
}

HOOK_RETURN_DATA DoomSetModel(edict_t* edict, const char* model) {
	static StringSet watched_ents = {
		"weaponbox"
	};

	if (watched_ents.hasKey(STRING(edict->v.classname))) {
		if (UTIL_ModelIsSprite(edict->v.modelindex)) {
			g_illuminate_ents.push_back(EHANDLE(edict));
		}
	}

	return HOOK_CONTINUE;
}

void update_sprite_brightness() {
	for (int i = 0; i < g_illuminate_ents.size(); i++) {
		CBaseEntity* ent = g_illuminate_ents[i];
		if (!ent) {
			g_illuminate_ents.erase(g_illuminate_ents.begin() + i);
			i--;
			continue;
		}

		ent->pev->rendercolor = g_bsp.get_lighting(ent->pev->origin).ApplyGamma().ToVector();
	}
}

extern "C" int DLLEXPORT PluginInit() {
	static HLCOOP_PLUGIN_HOOKS g_hooks;

	g_hooks.pfnPlayerUse = PlayerUse;
	g_hooks.pfnClientJoin = ClientJoin;
	g_hooks.pfnPlayerPostThink = PlayerPostThink;
	g_hooks.pfnPlayerTakeDamage = PlayerTakeDamage;
	g_hooks.pfnSpawnBlood = DoomBlood;
	g_hooks.pfnMapInit = MapInit;
	g_hooks.pfnMapStart = MapActivate;
	g_hooks.pfnSetModelPost = DoomSetModel;

	g_dmgScale = RegisterPluginCVar("dmg_scale", "1", 1, 0);

	RegisterPluginCommand(".idkfa", CmdGiveAll, FL_CMD_CLIENT | FL_CMD_ADMIN);
	RegisterPluginCommand(".score", CmdCheckScore, FL_CMD_CLIENT);

	RegisterPluginEntCallback(new_lobby_player);
	RegisterPluginEntCallback(ep_text);
	RegisterPluginEntCallback(intermission);
	RegisterPluginEntCallback(level_started);
	RegisterPluginEntCallback(secret_revealed);
	RegisterPluginEntCallback(player_killed);

	g_Scheduler.SetInterval(update_sprite_brightness, 0.05f , -1);

	return RegisterPlugin(&g_hooks);
}

extern "C" void DLLEXPORT PluginExit() {
	// nothing to clean up
}

class CNullEntity : public CBaseEntity
{
public:
	virtual int	GetEntindexPriority() { return ENTIDX_PRIORITY_LOW; }
	void Spawn(void) { UTIL_Remove(this); }
};

LINK_ENTITY_TO_CLASS(func_doom_door, CNullEntity)
LINK_ENTITY_TO_CLASS(func_doom_water, CNullEntity)

LINK_ENTITY_TO_CLASS(trigger_doom_teleport, CNullEntity)