#include "extdll.h"
#include "util.h"
#include "CItem.h"
#include "CBasePlayer.h"
#include "doom2.h"
#include "doom_utils.h"
#include "shake.h"
#include "Scheduler.h"
#include "CDoomMonster.h"
#include "te_effects.h"
#include "rgb.h"
#include "weapons.h"
#include "CWeaponCustom.h"

void invulnerability(EHANDLE h_plr, bool flicker)
{
	if (!h_plr.GetEntity())
		return;
		
	CBaseEntity* plr = h_plr;
	PlayerState& state = getPlayerState((CBasePlayer*)(plr));
		
	float timeLeft = state.godTimeLeft();
	if (timeLeft > 0 && plr->IsAlive())
	{
		Vector color(64, 255, 128);
		plr->pev->effects |= EF_BRIGHTLIGHT;
		plr->pev->takedamage = DAMAGE_NO;
		if (timeLeft < 5.0f && flicker)
		{
			color = Vector(255, 255, 255);
			plr->pev->effects &= ~EF_BRIGHTLIGHT;
		}
		flicker = !flicker;
		UTIL_ScreenFade(plr, color, 0.0f, 1.0f, 255, FFADE_MODULATE | FFADE_STAYOUT);
	}
	else
	{
		UTIL_ScreenFade(plr, Vector(255, 240, 64), 0.2f, 0, 0, FFADE_IN);
		plr->pev->effects &= ~EF_BRIGHTLIGHT;
		plr->pev->takedamage = DAMAGE_YES;
		return;
	}
		
	g_Scheduler.SetTimeout(invulnerability, 0.5f, h_plr, flicker);
}

void invisibility(EHANDLE h_plr, bool flicker)
{
	if (!h_plr.GetEntity())
		return;
		
	CBaseEntity* plr = h_plr;
	PlayerState& state = getPlayerState((CBasePlayer*)(plr));
		
	float timeLeft = state.invisTimeLeft();
	if (timeLeft > 0 && plr->IsAlive())
	{
		float renderAmt = 64;
		int renderMode = 2;
		if (timeLeft < 5.0f && flicker)
		{
			renderAmt = 255;
			renderMode = 2;
		}
		flicker = !flicker;
		plr->pev->rendermode = renderMode;
		plr->pev->renderamt = renderAmt;
	}
	else
	{
		plr->pev->rendermode = 0;
		plr->pev->renderamt = 0;
		return;
	}
		
	g_Scheduler.SetTimeout(invisibility, 0.5f, h_plr, flicker);
}

void suitprotect(EHANDLE h_plr, bool flicker)
{
	if (!h_plr.GetEntity())
		return;
		
	CBaseEntity* plr = h_plr;
	PlayerState& state = getPlayerState((CBasePlayer*)(plr));
	
	float timeLeft = state.suitTimeLeft();
	if (timeLeft > 0 && plr->IsAlive() && plr->pev->takedamage != DAMAGE_NO)
	{
		Vector color(0, 128, 0);
		if (timeLeft < 5.0f && flicker)
			color = Vector(0, 0, 0);
		flicker = !flicker;
		UTIL_ScreenFade(plr, color, 0.0f, 1.0f, 64, FFADE_STAYOUT);
	}
	else
	{
		UTIL_ScreenFade(plr, Vector(255, 240, 64), 0.2f, 0, 0, FFADE_IN);
		return;
	}
		
	g_Scheduler.SetTimeout(suitprotect, 0.5f, h_plr, flicker);
}

void goggles(EHANDLE h_plr, bool flicker)
{
	if (!h_plr.GetEntity())
		return;
		
	CBaseEntity* plr = h_plr;
	PlayerState& state = getPlayerState((CBasePlayer*)(plr));
		
	float timeLeft = state.goggleTimeLeft();
	if (timeLeft > 0 && plr->IsAlive())
	{
		plr->pev->effects |= EF_BRIGHTLIGHT;
		if (timeLeft < 5.0f && flicker)
			plr->pev->effects &= ~EF_BRIGHTLIGHT;
		flicker = !flicker;
	}
	else
	{
		plr->pev->effects &= ~EF_BRIGHTLIGHT;
		return;
	}
		
	g_Scheduler.SetTimeout(goggles, 0.5f, h_plr, flicker);
}

class CDoomBarrel : public CBaseEntity
{
public:
	int animFrameStart = 6;
	int animFrameMax = 7;

	bool dead = false;

	int animDir = 1;

	void Spawn()
	{
		// set the model we actually want
		//SET_MODEL(edict(), "models/doom/null.mdl");
		//SET_MODEL(edict(), "models/w_357.mdl");
		SET_MODEL(edict(), "sprites/doom/objects.spr");

		//pev->frame = 5;
		pev->scale = g_monster_scale;

		pev->movetype = MOVETYPE_PUSHSTEP;
		pev->solid = SOLID_BBOX;
		pev->health = 20;
		pev->takedamage = DAMAGE_YES;

		int light_level = Illumination();
		//println("ILLUM " + light_level);
		pev->rendercolor = Vector(light_level, light_level, light_level);

		UTIL_SetSize(pev, Vector(-16, -16, -4), Vector(16, 16, 40));
		pev->nextthink = gpGlobals->time;
		SetThink(&CDoomBarrel::Think);
	}

	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
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

			UTIL_DLight(pev->origin, 30, RGB(64, 40, 32), 3, 16);
		}

		return 0;
	}

	void Precache()
	{
		PRECACHE_MODEL("sprites/doom/objects.spr");
	}

	bool CustomPickup()
	{
		return false;
	}

	void Think()
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
};

class CDoomProp : public CBaseEntity
{
	int frameStart = 159;
	int frameMax = 159;
	int thing_type = 44;
	int animDir = 1;
	float animSpeed = 0.17f;
	
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
		// set the model we actually want
		//SET_MODEL(edict(), "models/doom/null.mdl");
		//SET_MODEL(edict(), "models/w_357.mdl");
		SET_MODEL( edict(), "sprites/doom/objects.spr");
		
		//pev->frame = 5;
		pev->scale = g_monster_scale;
		
		pev->movetype = MOVETYPE_NONE;
		pev->solid = SOLID_NOT;
		pev->takedamage = DAMAGE_NO;
		
		
		//println("ILLUM " + light_level);
		
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
		
		pev->rendercolor = fullBright ? Vector(1, 1, 1) : GetLighting().ToVector();
		pev->frame = frameStart;
	}
	
	void Precache()
	{
		PRECACHE_MODEL("sprites/doom/objects.spr");
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
};

void make_me_fall(EHANDLE h_ent)
{
	if (h_ent)
	{
		h_ent.GetEntity()->pev->movetype = MOVETYPE_TOSS;
	}
}

class CDoomItem : public CItem
{
public:
	int itemFrame = 0;
	int itemFrameMax = -1;
	float giveHealth = 0;
	float giveHealthMax = 100;
	float giveArmor = 0;
	float giveArmorMax = 200;
	bool giveGod = false;
	bool giveBerserk = false;
	bool giveInvis = false;
	bool giveSuit = false;
	bool giveGoggles = false;
	bool intermission = false;
	bool giveBackpack = false;
	string pickupSnd = "doom/dsitemup.wav";
	
	int animDir = 1;
	
	virtual void ItemSpawn()
	{
		pickupSnd = pickupSnd;
		
		// items spawn in the floor due to sven item code i guess
		pev->movetype = MOVETYPE_FLY;
		g_Scheduler.SetTimeout(make_me_fall, 1.0f, EHANDLE(edict()));
		
		// set the model we actually want
		SET_MODEL( edict(), "sprites/doom/objects.spr");
		CItem::Spawn();
		pev->frame = itemFrame;
		pev->scale = g_monster_scale;
		if (itemFrameMax == -1)
			itemFrameMax = itemFrame;
		
		//pev->movetype = MOVETYPE_NONE;
		//pev->solid = SOLID_NOT;
		
		pev->rendercolor = GetLighting().ApplyGamma().ToVector();
		
		UTIL_SetSize(pev, Vector(-8, -8, -4), Vector(8, 8, 8));
		
		if (itemFrameMax != itemFrame)
			pev->nextthink = gpGlobals->time;
	}
	
	void Precache()
	{
		PRECACHE_MODEL("sprites/doom/objects.spr");
	}
	
	virtual bool CustomPickup()
	{
		return false;
	}
	
	virtual void ItemThink() override
	{
		pev->frame += animDir;
		if (pev->frame > itemFrameMax)
		{
			pev->frame = itemFrameMax-1;
			animDir = -1;
		}
		if (pev->frame < itemFrame)
		{
			pev->frame = itemFrame+1;
			animDir = 1;
		}
		pev->nextthink = gpGlobals->time + 0.17f;
	}
	
	void Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
	{
		if (pActivator->IsPlayer())
		{
			TraceResult tr;
			UTIL_TraceLine( pev->origin, pActivator->pev->origin, dont_ignore_monsters, pActivator->edict(), &tr );
			if (tr.flFraction >= 1.0f)
				Touch(pActivator);
		}
	}
	
	void Touch( CBaseEntity* pOther )
	{
		if (!pOther->IsPlayer())
			return;
		
		CBasePlayer* plr = (CBasePlayer*)(pOther);
		PlayerState& state = getPlayerState(plr);
		
		bool pickedUp = CustomPickup();
		if (giveHealth > 0)
		{
			if (pOther->pev->health < giveHealthMax)
			{
				pickedUp = true;
				pOther->pev->health += giveHealth;
				if (pOther->pev->health > giveHealthMax)
					pOther->pev->health = giveHealthMax;
			}
		}
		if (giveArmor > 0)
		{
			if (pOther->pev->armorvalue < giveArmorMax)
			{
				pickedUp = true;
				pOther->pev->armorvalue += giveArmor;
				if (pOther->pev->armorvalue > giveArmorMax)
					pOther->pev->armorvalue = giveArmorMax;
			}
		}
		
		if (giveBackpack)
		{
			plr->GiveAmmo(10, "bullets");
			plr->GiveAmmo(4, "shells");
			plr->GiveAmmo(1, "rockets");
			plr->GiveAmmo(20, "cells");
			pickedUp = true;
		}
		
		if (giveGod)
		{
			if (state.godTimeLeft() <= 0)
				g_Scheduler.SetTimeout(invulnerability, 0.0f, EHANDLE(pOther->edict()), true);
			state.lastGod = gpGlobals->time;
			pickedUp = true;
		}
		if (giveBerserk)
		{
			CBasePlayerItem* item = plr->GetNamedPlayerItem("weapon_doom_fist");
			CWeaponCustom* cwep = item ? item->MyWeaponCustomPtr() : NULL;
			if (cwep)
			{
				plr->SelectItem("weapon_doom_fist");

				for (int i = 0; i < cwep->defaultParams.numEvents; i++) {
					WepEvt& evt = cwep->defaultParams.events[i];
					if (evt.evtType == WC_EVT_BULLETS) {
						evt.bullets.damage = 110;
						evt.bullets.damageRand = 90;
					}
				}

				pickedUp = true;
			}
			else {
				ALERT(at_warning, "No fist weapon to go berserk with!\n");
			}
		}
		if (giveInvis)
		{
			if (state.godTimeLeft() <= 0)
				g_Scheduler.SetTimeout(invisibility, 0.0f, EHANDLE(pOther->edict()), true);
			state.lastInvis = gpGlobals->time;
			pickedUp = true;
		}
		if (giveSuit)
		{
			if (state.suitTimeLeft() <= 0)
				g_Scheduler.SetTimeout(suitprotect, 0.0f, EHANDLE(pOther->edict()), true);
			state.lastSuit = gpGlobals->time;
			pickedUp = true;
		}
		if (giveGoggles)
		{
			if (state.goggleTimeLeft() <= 0)
				g_Scheduler.SetTimeout(goggles, 0.0f, EHANDLE(pOther->edict()), true);
			state.lastGoggles = gpGlobals->time;
			pickedUp = true;
		}
		
		if (pickedUp)
		{
			if (intermission)
				g_item_gets += 1;
			UTIL_ScreenFade(pOther, Vector(255, 240, 64), 0.2f, 0, 32, FFADE_IN);
			EMIT_SOUND_DYN(pOther->edict(), CHAN_ITEM, pickupSnd.c_str(), 1.0f, 0.5f, 0, 100);
			UTIL_Remove(this);
		}
		
		if (giveBerserk)
			UTIL_ScreenFade(pOther, Vector(255, 0, 0), 30.0f, 0, 32, FFADE_IN);
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
LINK_ENTITY_TO_CLASS(item_doom_medkit, CDoomMedkit)
LINK_ENTITY_TO_CLASS(item_doom_megaarmor, CDoomMegaArmor)
LINK_ENTITY_TO_CLASS(item_doom_megasphere, CDoomMegaSphere)
LINK_ENTITY_TO_CLASS(item_doom_potion, CDoomPotion)
LINK_ENTITY_TO_CLASS(item_doom_soulsphere, CDoomSoulSphere)
LINK_ENTITY_TO_CLASS(item_doom_stimpak, CDoomStimpak)
LINK_ENTITY_TO_CLASS(item_prop, CDoomProp)