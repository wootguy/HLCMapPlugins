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
#include "CDoomItem.h"

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


void CDoomItem::ItemSpawn()
{
	Precache();

	// set the model we actually want
	SET_MODEL(edict(), "sprites/doom/objects.spr");
	CItem::Spawn();
	pev->frame = itemFrame;
	pev->scale = g_monster_scale;
	if (itemFrameMax == -1)
		itemFrameMax = itemFrame;

	//pev->movetype = MOVETYPE_NONE;
	//pev->solid = SOLID_NOT;

	pev->rendercolor = GetLighting().ApplyGamma().ToVector();

	UTIL_SetSize(pev, Vector(-8, -8, -4), Vector(8, 8, 8));

	if (itemFrameMax == itemFrame)
		animDir = 0;
}

void CDoomItem::Precache()
{
	int modelIndexHw = PRECACHE_MODEL("sprites/doom/objects.spr");
	modelIndexSw = PRECACHE_MODEL("sprites/doom/sw/objects.spr");

	headerHw = GET_SPRITE_PTR(modelIndexHw);
	headerSw = GET_SPRITE_PTR(modelIndexSw);

	if (pickupSnd) {
		PRECACHE_SOUND(pickupSnd);
	}
}

bool CDoomItem::CustomPickup()
{
	return false;
}

void CDoomItem::ItemThink()
{
	pev->frame += animDir;
	if (pev->frame > itemFrameMax)
	{
		pev->frame = itemFrameMax - 1;
		animDir = -1;
	}
	if (pev->frame < itemFrame)
	{
		pev->frame = itemFrame + 1;
		animDir = 1;
	}
	pev->nextthink = gpGlobals->time + 0.17f;
}

void CDoomItem::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	if (pActivator->IsPlayer())
	{
		TraceResult tr;
		UTIL_TraceLine(pev->origin, pActivator->pev->origin, dont_ignore_monsters, pActivator->edict(), &tr);
		if (tr.flFraction >= 1.0f)
			Touch(pActivator);
	}
}

void CDoomItem::Touch(CBaseEntity* pOther)
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
		EMIT_SOUND_DYN(pOther->edict(), CHAN_ITEM, pickupSnd, 1.0f, 0.5f, 0, 100);
		UTIL_Remove(this);
	}

	if (giveBerserk)
		UTIL_ScreenFade(pOther, Vector(255, 0, 0), 30.0f, 0, 32, FFADE_IN);
}

int CDoomItem::AddToFullPack(struct entity_state_s* state, CBasePlayer* player) {
	if (player->m_clientRenderer == CLIENT_RENDERER_SOFTWARE) {
		state->origin.z += CDoomSprite::SwFrameOffset(headerHw, headerSw, pev->frame) * pev->scale;
		if (modelIndexSw) {
			state->modelindex = modelIndexSw;
		}
	}

	return 1;
}