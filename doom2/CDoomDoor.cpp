#include "extdll.h"
#include "util.h"
#include "CDoomDoor.h"
#include "Scheduler.h"
#include "doom2.h"

void reset_but(EHANDLE h_ent)
{
	if (!h_ent.GetEntity())
		return;
	CDoomDoor* ent = (CDoomDoor*)h_ent.GetEntity();
	ent->ButtonReset();
}

void delay_use(EHANDLE button, EHANDLE pActivator, int useType, float value, bool wasShot)
{
	if (!button.GetEntity() || !pActivator.GetEntity())
		return;
		
	CDoomDoor* ent = (CDoomDoor*)button.GetEntity();
	ent->Useit(pActivator.GetEntity(), pActivator.GetEntity(), USE_TYPE(useType), value, wasShot);
}


void CDoomDoor::KeyValue(KeyValueData* pkvd)
{	
	if (FStrEq(pkvd->szKeyName, "dir"))
	{
		dir = atoi(pkvd->szValue) == 1 ? 1 : -1;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "lip"))
	{
		m_flLip = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "wait"))
	{
		m_flWait = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "speed"))
	{
		pev->speed = atof(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "sounds"))
	{
		sounds = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "sync"))
	{
		sync = ALLOC_STRING(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "always_use"))
	{
		always_use = atoi(pkvd->szValue) != 0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "touch_opens"))
	{
		touch_opens = atoi(pkvd->szValue) != 0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "crusher"))
	{
		isCrusher = atoi(pkvd->szValue) != 0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "lock"))
	{
		lock = atoi(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "use_dir"))
	{
		useDir = UTIL_ParseVector(pkvd->szValue);
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "shootable"))
	{
		shootable = atoi(pkvd->szValue) != 0;
		pkvd->fHandled = TRUE;
	}
	else {
		CBaseEntity::KeyValue(pkvd);
	}
}
	
void CDoomDoor::Precache()
{
	PRECACHE_SOUND("doom/dsbdcls.wav"); // close quick
	PRECACHE_SOUND("doom/dsdorcls.wav"); // close
	PRECACHE_SOUND("doom/dsbdopn.wav"); // open quick
	PRECACHE_SOUND("doom/dsdoropn.wav"); // open
	PRECACHE_SOUND("doom/dsswtchn.wav"); // switch sound
	PRECACHE_SOUND("doom/dsswtchx.wav"); // switch sound2
	PRECACHE_SOUND("doom/dspstop.wav"); // floor stop
	PRECACHE_SOUND("doom/dspstart.wav"); // floor start
	PRECACHE_SOUND("doom/dsstnmov.wav"); // floor move
}
	
void CDoomDoor::Spawn()
{
	if (IsDelaySpawned())
		return; // prevent double-spawn bugs

	Precache();
		
	pev->movetype = MOVETYPE_PUSH;
	pev->solid = SOLID_BSP;
	pev->angles = g_vecZero;
	pev->takedamage = shootable ? DAMAGE_YES : DAMAGE_NO;
		
	UTIL_SetOrigin(pev, pev->origin);
	SET_MODEL(edict(), STRING(pev->model));
		
	if (pev->speed == 0)
		pev->speed = 100;
			
	if (isCrusher)
		m_flWait = 0.001f;
			
	m_vecPosition1	= pev->origin;
	// Subtract 2 from size because the engine expands bboxes by 1 in all directions making the size too big
	m_vecPosition2	= m_vecPosition1 + Vector(0,0,(dir * (pev->size.z-2)) - dir*m_flLip);
			
	isButton = pev->target;
	if (isButton)
	{
		if (sounds == 0)
			switchSnd = "doom/dsswtchn.wav";
		else
			switchSnd = "doom/dsswtchx.wav";
	}
	else
	{
		if (sounds == 0)
		{
			openSnd = "doom/dsdoropn.wav";
			closeSnd = "doom/dsdorcls.wav";
		}
		else if (sounds == 1)
		{
			openSnd = "doom/dsbdopn.wav";
			closeSnd = "doom/dsbdcls.wav";
		}
		else if (sounds == 2)
		{
			openSnd = "doom/dspstart.wav";
			closeSnd = "doom/dspstop.wav";
		}
		else if (sounds == 3)
		{
			openSnd = "doom/dsstnmov.wav";
			closeSnd = "doom/dspstop.wav";
		}
		else if (sounds == 4)
		{
			openSnd = "doom/dspstop.wav";
			closeSnd = "doom/dspstop.wav";
		}
	}
		
	switchSnd = switchSnd;
	openSnd = openSnd;
	closeSnd = closeSnd;

	if ( pev->spawnflags & SF_DOOR_START_OPEN != 0 )
	{	// swap pos1 && pos2, put door at pos2
		pev->origin = m_vecPosition2;
		m_vecPosition2 = m_vecPosition1;
		m_vecPosition1 = pev->origin;
	}

	m_toggle_state = TS_AT_BOTTOM;
		
	m_bIsReopening = false;
		
	// if the door is flagged for USE button activation only, use NULL touch function
	if ( (pev->spawnflags & SF_DOOR_USE_ONLY) == 0 )
		SetTouch( &CDoomDoor::Touch );
}
	
void CDoomDoor::Touch(CBaseEntity* other)
{
	if (isButton)
		return;
	if (m_toggle_state != TS_AT_BOTTOM && m_toggle_state != TS_AT_TOP)
		return;
			
	// Ignore touches by anything but players
	if (!other->IsPlayer())
		return;
		
	// If door is somebody's target, then touching does nothing.
	// You have to activate the owner (e.g. button).
	if (pev->targetname)
		return;
			
	if (!touch_opens)
		return; // never touch open
			
	DoorActivate();
}
	
int CDoomDoor::TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	if (!shootable)
		return 0;
	if ((bitsDamageType & DMG_BLAST) == 0)
	{
		CBaseEntity* activator = CBaseEntity::Instance( pevAttacker );
		//Useit(activator, activator, USE_TOGGLE, 0, true);
			
		// for some reason button won't activate if triggered now so have to wait a frame
		g_Scheduler.SetTimeout(delay_use, 0.0f, EHANDLE(edict()), EHANDLE(activator->edict()), int(USE_TOGGLE), 0, true);
	}
	return 0;
}
	
void CDoomDoor::ButtonReset()
{
	EMIT_SOUND_DYN(edict(), CHAN_STATIC, switchSnd, 1.0f, attn, 0, 100);
	pev->frame = 0;
}
	
void CDoomDoor::Useit(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value, bool wasShot)
{
	if (shootable && !wasShot)
		return;

	int haveKey = g_keys;
	if (!g_strict_keys)
	{
		// allow skull keys to activate normal-key doors && vice versa
		lock = (lock | (lock >> 3)) & (KEY_BLUE | KEY_YELLOW | KEY_RED);
		haveKey = g_keys | (g_keys >> 3);
	}
	if (lock != 0 && (lock & haveKey) != lock)
	{
		if (pActivator->IsPlayer())
		{
			const char* keyname = "blue";
			if (lock & (KEY_YELLOW | SKULL_YELLOW))
				keyname = "yellow";
			if (lock & (KEY_RED | SKULL_RED))
				keyname = "red";
			UTIL_ClientPrint(pActivator, print_center,
				UTIL_VarArgs("You need a %s key to activate this\n", keyname));
		}
		return;
	}
		
	if (pCaller->IsPlayer())
	{
		if (useDir != g_vecZero)
		{
			Vector doorOri = pev->absmin + (pev->absmax - pev->absmin)*0.5f;
			Vector delta = (doorOri - pCaller->pev->origin).Normalize();
			//println("USE DIR: " + useDir.ToString() + " == " + delta.ToString());
			if (DotProduct(delta, useDir) > 0)
				return;
		}
		//println("Z DELTA: " + (pev->absmax.z-pCaller->pev->absmin.z));
		if (pCaller->pev->absmin.z + 4 > pev->absmax.z)
		{
			return; // don't allow using floors from above (MAP05 secret)
		}
	}
		
	// if not ready to be used, ignore "use" command.
	if (isButton)
	{
		if (pev->frame == 0 && m_toggle_state == TS_AT_BOTTOM)
		{
			pev->frame = 1;
			EMIT_SOUND_DYN(edict(), CHAN_STATIC, switchSnd, 1.0f, attn, 0, 100);
			FireTargets(STRING(pev->target), pActivator, this, USE_TOGGLE);
			if (m_flWait > 0)
				g_Scheduler.SetTimeout(reset_but, m_flWait, EHANDLE(edict()));
		}
	}
	else if ((m_toggle_state == TS_AT_BOTTOM && useType != USE_OFF) || 
			(pev->spawnflags & SF_DOOR_NO_AUTO_RETURN) && m_toggle_state == TS_AT_TOP || useType == USE_OFF)
	{
		if ((!pev->targetname || always_use) || !pCaller->IsPlayer())
		{
			if (pev->targetname && always_use)
			{
				// door synced with others
				CBaseEntity* ent = NULL;
				do {
					ent = UTIL_FindEntityByTargetname(ent, STRING(pev->targetname));
					if (ent )
					{
						CDoomDoor* door = (CDoomDoor*)ent;
						door->DoorActivate(useType);
					}
				} while (ent );
			}
			else
				DoorActivate(useType);
		}
	}
}
	
void CDoomDoor::Use( CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value )
{
	Useit(pActivator, pCaller, useType, value, false);
}
	
int CDoomDoor::DoorActivate(int useType)
{
	if (m_flWait == -1)
	{
		if (m_toggle_state == TS_AT_TOP && (useType == USE_TOGGLE || useType == USE_ON))
			return 1;
	}
			
	if (isCrusher && useType == USE_OFF)
	{
		if (pev->nextthink != -1 && (sounds == 2 || sounds == 3 || sounds == 4) && closeSnd && closeSnd[0])
			EMIT_SOUND_DYN(edict(), CHAN_STATIC, closeSnd, 1.0f, attn, 0, 100);
		pev->nextthink = -1;
		return 1;
	}
			
	if ((pev->spawnflags & SF_DOOR_NO_AUTO_RETURN) && m_toggle_state == TS_AT_TOP || useType == USE_OFF)
	{
		if (m_toggle_state != TS_AT_BOTTOM && m_toggle_state != TS_GOING_DOWN)
			DoorGoDown();
	}
	else
		DoorGoUp();
			
	return 1;
}
	
void CDoomDoor::DoorGoUp()
{
	if (!isButton && openSnd && openSnd[0])
		EMIT_SOUND_DYN(edict(), CHAN_STATIC, openSnd, 1.0f, attn, 0, 100);
	m_toggle_state = TS_GOING_UP;
	LinearMove(m_vecPosition2, pev->speed);
		
	for (int i = 0; i < sync_buttons.size(); i++)
	{
		if (!sync_buttons[i].GetEntity())
			continue;
			
		CDoomDoor* but = (CDoomDoor*)sync_buttons[i].GetEntity();
		but->DoorGoUp();
	}
}
	
void CDoomDoor::DoorHitTop()
{
	if (openSnd)
		STOP_SOUND(edict(), CHAN_STATIC, openSnd);

	if ((sounds == 2 || sounds == 3 || sounds == 4) && closeSnd && closeSnd[0])
		EMIT_SOUND_DYN(edict(), CHAN_STATIC, closeSnd, 1.0f, attn, 0, 100);

	m_toggle_state = TS_AT_TOP;
	m_bIsReopening = false;
		
	// toggle-doors don't come down automatically, they wait for refire.
	if (!isButton && (pev->spawnflags & SF_DOOR_NO_AUTO_RETURN) == 0)
	{
		// In flWait seconds, DoorGoDown will fire, unless wait is -1, then door stays open
		pev->nextthink = pev->ltime + m_flWait;				
		SetThink( &CDoomDoor::DoorGoDown );

		if ( m_flWait == -1 )
			pev->nextthink = -1;
	}
}
	
void CDoomDoor::DoorGoDown()
{
	if (!isButton)
	{
		if (sounds == 2 || sounds == 3)
		{
			if (openSnd && openSnd[0])
				EMIT_SOUND_DYN(edict(), CHAN_STATIC, openSnd, 1.0f, attn, 0, 100);
		}
		else if (closeSnd && closeSnd[0])
			EMIT_SOUND_DYN(edict(), CHAN_STATIC, closeSnd, 1.0f, attn, 0, 100);
	}
	m_toggle_state = TS_GOING_DOWN;
	LinearMove( m_vecPosition1, pev->speed);
		
	for (int i = 0; i < sync_buttons.size(); i++)
	{
		if (!sync_buttons[i].GetEntity())
			continue;
			
		CDoomDoor* but = (CDoomDoor*)sync_buttons[i].GetEntity();
		but->DoorGoDown();
	}
}

void CDoomDoor::DoorHitBottom()
{
	if (openSnd)
		STOP_SOUND(edict(), CHAN_STATIC, openSnd);

	if ((sounds == 2 || sounds == 3) && closeSnd && closeSnd[0])
		EMIT_SOUND_DYN(edict(), CHAN_STATIC, closeSnd, 1.0f, attn, 0, 100);
	m_toggle_state = TS_AT_BOTTOM;
		
	if (isCrusher)
		DoorActivate();
}
	
void CDoomDoor::Blocked( CBaseEntity* pOther )
{
	if (isButton && parent.GetEntity())
	{
		parent.GetEntity()->Blocked(pOther);
		return;
	}
		
	// Hurt the blocker a little.
	if ( pev->dmg != 0 && lastCrush + 0.0572f < gpGlobals->time)
	{
		lastCrush = gpGlobals->time;
		pOther->TakeDamage( pev, pev, pev->dmg, DMG_CRUSH );
		DoomBlood(pOther->pev->origin + pOther->pev->view_ofs);
	}

	// if a door has a negative wait, it would never come back if blocked,
	// so let it just squash the object to death real fast

	if (m_flWait >= 0 && !isCrusher)
	{
		if (m_toggle_state == TS_GOING_DOWN)
			DoorGoUp();
		else
			DoorGoDown();
	}
}

void CDoomDoor::LinearMove(Vector vecDest, float flSpeed)
{
	m_vecFinalDest = vecDest;
		
	// Already there?
	if (vecDest == pev->origin)
	{
		LinearMoveDone();
		return;
	}
			
	// set destdelta to the vector needed to move
	Vector vecDestDelta = vecDest - pev->origin;
		
	// divide vector length by speed to get time to reach dest
	float flTravelTime = vecDestDelta.Length() / flSpeed;

	// set nextthink to trigger a call to LinearMoveDone when dest is reached
	pev->nextthink = pev->ltime + flTravelTime;
	SetThink( &CDoomDoor::LinearMoveDone );

	// scale the destdelta vector by the time spent traveling to get velocity
	pev->velocity = vecDestDelta / flTravelTime;
}
	
void CDoomDoor::LinearMoveDone()
{
	Vector delta = m_vecFinalDest - pev->origin;
	float error = delta.Length();
	if ( error > 0.03125 )
	{
		LinearMove( m_vecFinalDest, 100 );
		return;
	}

	pev->origin = m_vecFinalDest;
	pev->velocity = g_vecZero;
	pev->nextthink = -1;
		
	if (m_toggle_state == TS_GOING_UP)
		DoorHitTop();
	else
		DoorHitBottom();
}
	
void CDoomDoor::SetToggleState( int state )
{
	if ( state == TS_AT_TOP )
		UTIL_SetOrigin( pev, m_vecPosition2);
	else
		UTIL_SetOrigin(pev, m_vecPosition1 );
}

LINK_ENTITY_TO_CLASS(func_doom_door, CDoomDoor)