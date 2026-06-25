#include "extdll.h"
#include "util.h"
#include "CDoomTeleport.h"
#include "CDoomMonster.h"
#include "CBasePlayer.h"
#include "te_effects.h"
#include "Scheduler.h"

using namespace std;

unordered_map<int, unordered_map<int, float>> g_last_touches;

void delay_tele_effect(Vector pos)
{
	UTIL_ExplosionMsg(pos, MODEL_INDEX("sprites/doom/tfog.spr"), 10, 5, 15);
}

void CDoomTeleport::KeyValue(KeyValueData* pkvd)
{		
	if (FStrEq(pkvd->szKeyName, "tele_dir"))
	{
		teleDir = UTIL_ParseVector(pkvd->szValue).Normalize();
		pkvd->fHandled = TRUE;
	}
	else {
		CBaseEntity::KeyValue(pkvd);
	}
}
	
void CDoomTeleport::Spawn()
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	Precache();
	pev->movetype = MOVETYPE_NONE;
	pev->solid = SOLID_TRIGGER;
	pev->effects = EF_NODRAW;
	touchDelay = 0.5f;
		
	ignore_players = (pev->spawnflags & FL_TP_IGNORE_PLAYERS) != 0;
	tele_on_exit = (pev->spawnflags & FL_TP_ON_EXIT) != 0;
		
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(edict(), STRING(pev->model));
		
	SetTouch( &CDoomTeleport::Touch );

	unordered_map<int, float>& lastTouched = g_last_touches[entindex()];
	lastTouched.clear();
}

void CDoomTeleport::Precache()
{
	PRECACHE_MODEL("sprites/doom/tfog.spr");
	PRECACHE_SOUND("doom/dstelept.wav");
}
	
void CDoomTeleport::Teleport(CBaseEntity* other)
{
	unordered_map<int, float>& lastTouches = g_last_touches[entindex()];

	CBaseEntity* target = UTIL_FindEntityByTargetname(NULL, STRING(pev->target));
	if (target )
	{
		Vector offset = other->IsPlayer() ? Vector(0,0,36) : Vector(0,0,0);
		Vector targetPos = target->pev->origin + offset;
		Vector testPos = target->pev->origin + Vector(0,0,36);
			
		// telefrag
		TraceResult tr;
		UTIL_TraceHull( testPos, testPos + Vector(0,0,1.0f), dont_ignore_monsters, human_hull, edict(), &tr );

		CBaseEntity* phit = CBaseEntity::Instance( tr.pHit );
		if (phit  && (phit->IsMonster() || phit->IsPlayer()))
			phit->TakeDamage(other->pev, other->pev, phit->pev->health + 100, DMG_CRUSH);
				
		if (other->IsMonster())
		{
			// TODO: need a safe cast here
			CDoomMonster* mon = (CDoomMonster*)other;
			if (mon)
				mon->DelayAttack(); // get out of the way for other monsters
		}
			
		UTIL_ExplosionMsg(other->pev->origin - offset, MODEL_INDEX("sprites/doom/tfog.spr"), 10, 5, 15);
		UTIL_SetOrigin(other->pev, target->pev->origin + offset);
			
		CBaseEntity* ent = NULL;
		do {
			ent = UTIL_FindEntityByClassname(ent, "trigger_doom_teleport");
			if (ent )
			{
				if (ent->Intersects(other))
				{
					// if ent will land inside another teleport trigger, then prevent it 
					// from teleporting without re-entering the brush
					CDoomTeleport* tele = (CDoomTeleport*)ent;
					lastTouches[other->entindex()] = gpGlobals->time;
				}
			}
		} while (ent);
			
		g_engfuncs.pfnMakeVectors(target->pev->angles);
		g_Scheduler.SetTimeout(delay_tele_effect, 0.05f, other->pev->origin - offset + gpGlobals->v_forward*32);
			
		EMIT_SOUND_DYN(edict(), CHAN_STATIC, "doom/dstelept.wav", 1.0f, 1.0f, 0, 100);
		EMIT_SOUND_DYN(target->edict(), CHAN_STATIC, "doom/dstelept.wav", 1.0f, 1.0f, 0, 100);
			
		other->pev->velocity = Vector(0,0,0);
		other->pev->angles = target->pev->angles;
		other->pev->v_angle = target->pev->angles;
		other->pev->fixangle = FAM_FORCEVIEWANGLES;
	}
	else
		ALERT(at_error, "Bad teleport destination: %s", STRING(pev->target));
}
	
void CDoomTeleport::Touch(CBaseEntity* other)
{
	if (!other->IsMonster() && !other->IsPlayer())
		return;
			
	if (ignore_players && other->IsPlayer())
		return;
	
	unordered_map<int, float>& lastTouches = g_last_touches[entindex()];

	bool newToucher = true;
	float lastTouch = -1;
	if (lastTouches.count(other->entindex()))
		lastTouch = lastTouches[other->entindex()];
	lastTouches[other->entindex()] = gpGlobals->time;
		
	float diff = gpGlobals->time - lastTouch;
	bool hasDir = teleDir != g_vecZero;
	if (diff > touchDelay && !tele_on_exit || hasDir)
	{
		bool validDir = false;
		if (hasDir)
		{
			Vector moveDir;
			if (other->IsPlayer())
				moveDir = other->pev->velocity.Normalize();
			else
				moveDir = (other->pev->origin - other->pev->oldorigin).Normalize();
			validDir = DotProduct(moveDir, teleDir) > 0.1;
			//println("DOT: " + DotProduct(moveDir, teleDir) + " " + validDir + " " + other.IsMonster());
		}
		if (!hasDir || validDir)
		{
			// just started touching (fire on enter)
			Teleport(other);
		}	
	}
}

LINK_ENTITY_TO_CLASS(trigger_doom_teleport, CDoomTeleport)