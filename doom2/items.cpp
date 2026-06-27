#include "extdll.h"
#include "util.h"
#include "CItem.h"
#include "doom2.h"
#include "doom_utils.h"
#include "Scheduler.h"
#include "CDoomMonster.h"
#include "te_effects.h"
#include "rgb.h"
#include "weapons.h"
#include "CDoomItem.h"
#include "CBasePlayer.h"

class CDoomBarrel : public CBaseEntity
{
public:
	int animFrameStart = 6;
	int animFrameMax = 7;
	int modelIndexSw;

	bool dead = false;

	int animDir = 1;

	void Spawn() override
	{
		Precache();

		if (!g_map_init_done) {
			UTIL_Remove(this);
			return;
		}

		// set the model we actually want
		//SET_MODEL(edict(), "models/doom/null.mdl");
		//SET_MODEL(edict(), "models/w_357.mdl");
		SET_MODEL(edict(), "sprites/doom/objects.spr");

		//pev->frame = 5;
		pev->scale = g_monster_scale;

		pev->movetype = MOVETYPE_TOSS;
		pev->solid = SOLID_BBOX;
		pev->health = 20;
		pev->takedamage = DAMAGE_YES;

		pev->rendercolor = GetLighting().ApplyGamma().ToVector();

		UTIL_SetSize(pev, Vector(-16, -16, -4), Vector(16, 16, 40));
		pev->nextthink = gpGlobals->time;
		SetThink(&CDoomBarrel::DropThink);
	}

	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) override
	{
		if (dead)
			return 0;
		Vector delta = (pevAttacker->origin - pev->origin).Normalize();
		pev->basevelocity = delta * -128;

		pev->health -= flDamage;

		if (pev->health <= 0)
		{
			dead = true;
			pev->frame = 9;
			animFrameStart = 9;
			animFrameMax = 11;
			animDir = 1;
			pev->nextthink = gpGlobals->time + 0.17f;
			EMIT_SOUND_DYN(edict(), CHAN_STATIC, "doom/dsbarexp.wav", 1.0f, 1.0f, 0, 100);

			pev->rendercolor.x = V_max(pev->rendercolor.x, 200);
			pev->rendercolor.y = V_max(pev->rendercolor.y, 160);
			pev->rendercolor.z = V_max(pev->rendercolor.z, 100);

			UTIL_DLight(pev->origin, 30, RGB(64, 40, 32), 3, 16);
		}

		return 0;
	}

	void Precache() override
	{
		PRECACHE_MODEL("sprites/doom/objects.spr");
		modelIndexSw = PRECACHE_MODEL("sprites/doom/sw/objects.spr");
	}

	void DropThink() {
		TraceResult tr;
		UTIL_TraceHull(pev->origin, pev->origin - Vector(0, 0, 2048), ignore_monsters, head_hull, edict(), &tr);

		if (tr.flFraction < 1.0f) {
			UTIL_SetOrigin(pev, tr.vecEndPos);
		}

		SetThink(&CDoomBarrel::BarrelThink);
		pev->nextthink = gpGlobals->time;
	}

	void BarrelThink()
	{
		pev->frame += animDir;
		if (dead && pev->frame > animFrameMax)
		{
			UTIL_Remove(this);
			return;
		}
		if (pev->frame > animFrameMax)
		{
			pev->frame = animFrameMax - 1;
			animDir = -1;
		}
		if (pev->frame < animFrameStart)
		{
			pev->frame = animFrameStart + 1;
			animDir = 1;
		}
		if (dead && pev->frame == 11)
		{
			RadiusDamage(pev->origin, pev, pev, 128, 128 * g_monster_scale, 0, DMG_BLAST);
		}
		pev->nextthink = gpGlobals->time + 0.17f;
	}

	int AddToFullPack(struct entity_state_s* state, CBasePlayer* player) override {
		if (player->m_clientRenderer == CLIENT_RENDERER_SOFTWARE) {
			state->origin.z += 19;
			if (modelIndexSw) {
				state->modelindex = modelIndexSw;
			}
		}

		return 1;
	}
};

class CDoomProp : public CBaseEntity
{
	int frameStart = 159;
	int frameMax = 159;
	int thing_type = 44;
	int animDir = 1;
	float animSpeed = 0.17f;
	int modelIndexSw;
	
	void KeyValue(KeyValueData* pkvd)
	{		
		if (FStrEq(pkvd->szKeyName, "thing_type"))
		{
			thing_type = atoi(pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
		else {
			CBaseEntity::KeyValue(pkvd);
		}
	}
	
	void Spawn()
	{
		Precache();

		if (!g_map_init_done) {
			UTIL_Remove(this);
			return;
		}

		// set the model we actually want
		//SET_MODEL(edict(), "models/doom/null.mdl");
		//SET_MODEL(edict(), "models/w_357.mdl");
		SET_MODEL( edict(), "sprites/doom/hw/objects.spr");
		
		//pev->frame = 5;
		pev->scale = g_monster_scale;
		
		pev->movetype = MOVETYPE_NONE;
		pev->solid = SOLID_NOT;
		pev->takedamage = DAMAGE_NO;
		
		setFrames();
		
		if (frameStart != frameMax)
		{
			pev->nextthink = gpGlobals->time;
			SetThink(&CDoomProp::Think);
		}
	}
	
	void setFrames()
	{
		bool fullBright = false;
		
		switch(thing_type)
		{
			case 24:
				frameStart = frameMax = 107;
				break;
			case 26:
				frameStart = 108;
				frameMax = 109;
				animSpeed = 0.2f;
				break;
			case 27:
				frameStart = frameMax = 106;
				break;
			case 25:
				frameStart = frameMax = 102;
				break;
			case 30:
				frameStart = frameMax = 38;
				break;
			case 31:
				frameStart = frameMax = 39;
				break;
			case 34:
				frameStart = frameMax = 30;
				fullBright = 255;
				break;
			case 43:
				frameStart = frameMax = 158;
				break;
			case 44:
				frameStart = 142;
				frameMax = 145;
				fullBright = 255;
				break;
			case 45:
				frameStart = 146;
				frameMax = 149;
				fullBright = 255;
				break;
			case 46:
				frameStart = 160;
				frameMax = 163;
				fullBright = 255;
				break;
			case 47:
				frameStart = frameMax = 130;
				break;
			case 53:
				frameStart = frameMax = 60;
				break;
			case 54:
				frameStart = frameMax = 159;
				break;
			case 55:
				frameStart = 122;
				frameMax = 125;
				fullBright = 255;
				break;
			case 56:
				frameStart = 126;
				frameMax = 129;
				fullBright = 255;
				break;
			case 57:
				frameStart = 131;
				frameMax = 134;
				fullBright = 255;
				break;
			case 70:
				frameStart = 48;
				frameMax = 50;
				fullBright = 255;
				break;
			case 75:
				frameStart = frameMax = 63;
				break;
			case 78:
				frameStart = frameMax = 66;
				break;
			case 79: case 80:
				frameStart = 100;
				frameMax = 100;
				break;
			case 85:
				frameStart = 150;
				frameMax = 153;
				fullBright = 255;
				break;
			case 86:
				frameStart = 154;
				frameMax = 157;
				fullBright = 255;
				break;
			default:
				ALERT(at_error, "Unhandled prop type: %d", thing_type);
				break;
		}
		
		pev->rendercolor = fullBright ? Vector(255, 255, 255) : GetLighting().ApplyGamma().ToVector();
		pev->frame = frameStart;
	}
	
	void Precache()
	{
		PRECACHE_MODEL("sprites/doom/hw/objects.spr");
		modelIndexSw = PRECACHE_MODEL("sprites/doom/sw/objects.spr");
	}
	
	bool CustomPickup()
	{
		return false;
	}
	
	void Think()
	{		
		pev->frame += animDir;
		if (pev->frame > frameMax)
			pev->frame = frameStart;
		pev->nextthink = gpGlobals->time + animSpeed;
	}

	int AddToFullPack(struct entity_state_s* state, CBasePlayer* player) override {
		if (player->m_clientRenderer == CLIENT_RENDERER_SOFTWARE) {
			state->origin.z += 32;
			if (modelIndexSw) {
				state->modelindex = modelIndexSw;
			}
		}

		return 1;
	}
};

class CDoomBackpack : public CDoomItem
{
	void Spawn() override
	{
		giveBackpack = true;
		itemFrame = 25;
		ItemSpawn();
	}
};

class CDoomStimpak : public CDoomItem
{
	void Spawn()
	{
		giveHealth = 10;
		itemFrame = 140;
		ItemSpawn();
	}
};

class CDoomMedkit : public CDoomItem
{
	void Spawn()
	{
		giveHealth = 25;
		itemFrame = 81;
		ItemSpawn();
	}
};

class CDoomPotion : public CDoomItem
{
	void Spawn()
	{
		giveHealth = 1;
		giveHealthMax = 200;
		itemFrame = 17;
		itemFrameMax = 20;
		intermission = true;
		ItemSpawn();
	}
};

class CDoomArmorBonus : public CDoomItem
{
	void Spawn()
	{
		giveArmor = 1;
		giveArmorMax = 200;
		itemFrame = 21;
		itemFrameMax = 23;
		intermission = true;
		ItemSpawn();
	}
};

class CDoomArmor : public CDoomItem
{
	void Spawn()
	{
		giveArmor = 100;
		giveArmorMax = 100;
		itemFrame = 1;
		itemFrameMax = 2;
		ItemSpawn();
	}
};

class CDoomMegaArmor : public CDoomItem
{
	void Spawn()
	{
		giveArmor = 200;
		giveArmorMax = 200;
		itemFrame = 3;
		itemFrameMax = 4;
		ItemSpawn();
	}
};

class CDoomMegaSphere : public CDoomItem
{
	void Spawn()
	{
		giveHealth = 200;
		giveHealthMax = 200;
		giveArmor = 200;
		giveArmorMax = 200;
		itemFrame = 82;
		itemFrameMax = 85;
		intermission = true;
		pickupSnd = "doom/dsgetpow.wav";
		ItemSpawn();
	}
};

class CDoomSoulSphere : public CDoomItem
{
	void Spawn()
	{
		giveHealth = 100;
		giveHealthMax = 200;
		itemFrame = 136;
		itemFrameMax = 139;
		pickupSnd = "doom/dsgetpow.wav";
		ItemSpawn();
	}
};

class CDoomGod : public CDoomItem
{
	void Spawn()
	{
		giveGod = true;
		itemFrame = 91;
		itemFrameMax = 94;
		intermission = true;
		pickupSnd = "doom/dsgetpow.wav";
		ItemSpawn();
	}
};

class CDoomBerserk : public CDoomItem
{
	void Spawn()
	{
		giveHealth = 100;
		giveHealthMax = 100;
		giveBerserk = true;
		itemFrame = 110;
		intermission = true;
		pickupSnd = "doom/dsgetpow.wav";
		ItemSpawn();
	}
};

class CDoomInvis : public CDoomItem
{
	void Spawn()
	{
		giveInvis = true;
		itemFrame = 87;
		itemFrameMax = 90;
		intermission = true;
		pickupSnd = "doom/dsgetpow.wav";
		ItemSpawn();
	}
};

class CDoomSuit : public CDoomItem
{
	void Spawn()
	{
		giveSuit = true;
		itemFrame = 141;
		itemFrameMax = 141;
		pickupSnd = "doom/dsgetpow.wav";
		ItemSpawn();
	}
};

class CDoomGoggles : public CDoomItem
{
	void Spawn()
	{
		giveGoggles = true;
		itemFrame = 111;
		itemFrameMax = 112;
		intermission = true;
		pickupSnd = "doom/dsgetpow.wav";
		ItemSpawn();
	}
};

class CDoomKeyRed : public CDoomItem
{
	void Spawn()
	{
		itemFrame = 113;
		itemFrameMax = 114;
		ItemSpawn();
	}

	bool CustomPickup() override
	{
		UTIL_ClientPrintAll(print_center, "Picked up a red keycard");
		g_keys |= KEY_RED;
		return true;
	}
};

class CDoomKeyBlue : public CDoomItem
{
	void Spawn()
	{
		itemFrame = 15;
		itemFrameMax = 16;
		ItemSpawn();
	}

	virtual void ItemThink() override {
		CDoomItem::ItemThink();
	}

	bool CustomPickup()
	{
		UTIL_ClientPrintAll(print_center, "Picked up a blue keycard");
		g_keys |= KEY_BLUE;
		return true;
	}
};

class CDoomKeyYellow : public CDoomItem
{
	void Spawn()
	{
		itemFrame = 164;
		itemFrameMax = 165;
		ItemSpawn();
	}

	bool CustomPickup()
	{
		UTIL_ClientPrintAll(print_center, "Picked up a yellow keycard");
		g_keys |= KEY_YELLOW;
		return true;
	}
};

class CDoomSkullRed : public CDoomItem
{
	void Spawn()
	{
		itemFrame = 116;
		itemFrameMax = 117;
		ItemSpawn();
	}

	bool CustomPickup()
	{
		UTIL_ClientPrintAll(print_center, "Picked up a red skull key");
		g_keys |= SKULL_RED;
		return true;
	}
};

class CDoomSkullBlue : public CDoomItem
{
	void Spawn()
	{
		itemFrame = 28;
		itemFrameMax = 29;
		ItemSpawn();
	}

	bool CustomPickup()
	{
		UTIL_ClientPrintAll(print_center, "Picked up a blue skull key");
		g_keys |= SKULL_BLUE;
		return true;
	}
};

class CDoomSkullYellow : public CDoomItem
{
	void Spawn()
	{
		itemFrame = 166;
		itemFrameMax = 167;
		ItemSpawn();
	}

	bool CustomPickup()
	{
		UTIL_ClientPrintAll(print_center, "Picked up a yellow skull key");
		g_keys |= SKULL_YELLOW;
		return true;
	}
};

LINK_ENTITY_TO_CLASS(item_barrel, CDoomBarrel)
LINK_ENTITY_TO_CLASS(item_doom_armor, CDoomArmor)
LINK_ENTITY_TO_CLASS(item_doom_armor_bonus, CDoomArmorBonus)
LINK_ENTITY_TO_CLASS(item_doom_backpack, CDoomBackpack)
LINK_ENTITY_TO_CLASS(item_doom_berserk, CDoomBerserk)
LINK_ENTITY_TO_CLASS(item_doom_invis, CDoomInvis)
LINK_ENTITY_TO_CLASS(item_doom_key_blue, CDoomKeyBlue)
LINK_ENTITY_TO_CLASS(item_doom_key_red, CDoomKeyRed)
LINK_ENTITY_TO_CLASS(item_doom_key_yellow, CDoomKeyYellow)
LINK_ENTITY_TO_CLASS(item_doom_skull_blue, CDoomSkullBlue)
LINK_ENTITY_TO_CLASS(item_doom_skull_red, CDoomSkullRed)
LINK_ENTITY_TO_CLASS(item_doom_skull_yellow, CDoomSkullYellow)
LINK_ENTITY_TO_CLASS(item_doom_medkit, CDoomMedkit)
LINK_ENTITY_TO_CLASS(item_doom_megaarmor, CDoomMegaArmor)
LINK_ENTITY_TO_CLASS(item_doom_megasphere, CDoomMegaSphere)
LINK_ENTITY_TO_CLASS(item_doom_potion, CDoomPotion)
LINK_ENTITY_TO_CLASS(item_doom_soulsphere, CDoomSoulSphere)
LINK_ENTITY_TO_CLASS(item_doom_stimpak, CDoomStimpak)
LINK_ENTITY_TO_CLASS(item_doom_suit, CDoomSuit)
LINK_ENTITY_TO_CLASS(item_doom_enviro, CDoomSuit)
LINK_ENTITY_TO_CLASS(item_doom_god, CDoomGod)
LINK_ENTITY_TO_CLASS(item_doom_goggles, CDoomGoggles)
LINK_ENTITY_TO_CLASS(item_prop, CDoomProp)