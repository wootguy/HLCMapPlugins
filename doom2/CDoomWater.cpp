#include "extdll.h"
#include "util.h"
#include "CDoomWater.h"
#include "Scheduler.h"
#include "doom2.h"
#include "doom_utils.h"

void weird_think_bug_workaround(EHANDLE h_ent)
{
	if (!h_ent.GetEntity())
		return;
	CDoomWater* ent = (CDoomWater*)h_ent.GetEntity();
	
	ent->WaterThink();
}

void CDoomWater::KeyValue(KeyValueData* pkvd)
{		
	if (FStrEq(pkvd->szKeyName, "damage"))
	{
		damage = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "maxFrame"))
	{
		maxFrame = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else {
		CBaseEntity::KeyValue(pkvd);
	}
}
	
void CDoomWater::Spawn()
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	if (!maxFrame) {
		maxFrame = 2;
	}
	pev->movetype = MOVETYPE_PUSH;
	pev->solid = SOLID_BSP;
	//pev->effects = EF_FRAMEANIMTEXTURES;
	// TODO: anim textures
		
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(edict(), STRING(pev->model));
		
	SetTouch( &CDoomWater::Touch );
	SetThink( &CDoomWater::WaterThink );
	WaterThink();
}
	
void CDoomWater::WaterThink()
{
	pev->frame += 1;
	if (pev->frame > maxFrame)
		pev->frame = 0;
	g_Scheduler.SetTimeout(weird_think_bug_workaround, 0.25f, EHANDLE(edict()));
		
}
	
void CDoomWater::Touch(CBaseEntity* other)
{		
	if (!other->IsPlayer() || !other->IsAlive())
		return;
			
	int idx = other->entindex();
	PlayerState& state = getPlayerState((CBasePlayer*)(other));
		
	float lastTouch = lastTouches[idx];
	float lastPain = lastPains[idx];
		
	float diff = gpGlobals->time - lastTouch;
	if (diff > 0.5f)
	{
		lastPain = gpGlobals->time - 0.5f; // player just started touching, don't hurt yet
	}
	else
	{
		diff = gpGlobals->time - lastPain;
		if (damage != 0 && state.suitTimeLeft() <= 0 && diff > 0.8f)
		{
			other->TakeDamage(pev, pev, damage, DMG_ACID);
			lastPain = gpGlobals->time;
		}
	}
		
	lastPains[idx] = lastPain;
	lastTouches[idx] = gpGlobals->time;
}

LINK_ENTITY_TO_CLASS(func_doom_water, CDoomWater)