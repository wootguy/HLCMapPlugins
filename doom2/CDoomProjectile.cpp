#include "extdll.h"
#include "util.h"
#include "CDoomProjectile.h"
#include "doom_utils.h"
#include "te_effects.h"
#include "CDoomMonster.h"
#include "weapons.h"

void CDoomProjectile::Spawn()
{
	CDoomSprite::Spawn();
	Precache();

	if (!size)
		size = 4; // too big and projectiles hit walls behind you that you're touching

	pev->movetype = MOVETYPE_FLY;
	pev->solid = SOLID_TRIGGER;
	UTIL_SetSize(pev, Vector(-size, -size, -size), Vector(size, size, size));

	pev->angles.x *= -1; // TODO: ???
	UTIL_MakeVectors(pev->angles);
	pev->velocity = gpGlobals->v_forward*pev->speed*g_monster_scale;
	lastVelocity = pev->velocity;

	if (spawnSound) {
		if (is_vile_fire) {
			ALERT(at_error, "TODO: Vile fire sound loop\n");
		}
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, spawnSound, 1.0f, 0.5f, 0, 100);
	}
		
	if (is_vile_fire)
		deathTime = gpGlobals->time + 2.2f;
		
	pev->scale = g_monster_scale;
	pev->frame = moveFrameStart;
	pev->targetname = ALLOC_STRING(UTIL_VarArgs("m%d", ++g_monster_idx));

	SetThink( &CDoomProjectile::Think );
	pev->nextthink = gpGlobals->time;
}

void CDoomProjectile::Precache()
{
	CDoomSprite::Precache();

	if (spawnSound)
		PRECACHE_SOUND(spawnSound);
	if (deathSound)
		PRECACHE_SOUND(deathSound);
	if (trailSprite)
		PRECACHE_MODEL(trailSprite);
}
	
void CDoomProjectile::Remove()
{
	STOP_SOUND(edict(), CHAN_WEAPON, spawnSound);
	UTIL_Remove(this);
}
	
void CDoomProjectile::Touch( CBaseEntity* pOther )
{
	//if (dead || (pOther->pev->classname == "fireball"))
	if (dead)
		return;

	if (pOther->edict() == pev->owner || pOther->pev->solid == SOLID_TRIGGER) {
		return; // don't touch owner or other projectiles
	}
			
	if (is_vile_fire)
	{
		if (!FireThink())
		{
			Remove();
			return;
		}
		STOP_SOUND(edict(), CHAN_WEAPON, spawnSound);
		is_vile_fire = false; // back to the normal think code
	}
			
	CBaseEntity* owner = CBaseEntity::Instance( pev->owner );
			
	dead = true;
	pev->solid = SOLID_NOT;
	pev->frame = deathFrameStart;
	frameCounter = deathFrameStart;
	oriented = false;
	pev->nextthink = gpGlobals->time + 0.15;
	pev->rendermode = 0;
	pev->effects &= ~EF_NODRAW;
	pev->movetype = MOVETYPE_NONE;
		
	int damage = RANDOM_LONG(damageMin, damageMax);
	Vector oldVel = pOther->pev->velocity;
	pOther->TakeDamage(pev, owner ? owner->pev : pev, damage, DMG_GENERIC);
	pOther->pev->velocity = oldVel; // prevent vertical launching
	knockBack(pOther, pev->velocity.Normalize()*(100 + damage*2));
		
	if (radiusDamage > 0)
		RadiusDamage(pev->origin, pev, owner ? owner->pev : pev, radiusDamage, radiusDamage*g_monster_scale, 0, DMG_BLAST);
		
	if (is_bfg && owner )
	{
		// do weird bfg tracer stuff
		float range = 1024;
		float spread = 90.0f;
		Vector dir = pev->velocity.Normalize();
		Vector vecSrc = owner->pev->origin;
		unordered_set<int> targets;
			
		for (int i = 0; i < 40 * 4; i++) // original is 40 traces. Added more to account for vertical spread
		{
			Vector vecAiming = spreadDir(dir, spread, SPREAD_UNIFORM);

			// Do the bullet collision
			TraceResult tr;
			Vector vecEnd = vecSrc + vecAiming * range;
			UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, owner->edict(), &tr );
			//te_debug_beam(vecSrc, vecEnd, 1.0f);
				
			// do more fancy effects
			if( tr.flFraction < 1.0 )
			{
				CBaseEntity* pHit = CBaseEntity::Instance( tr.pHit );
					
				if (pHit && !pHit->IsBSPModel()) 
				{
					// Actual damage is 1-8 16x. Averaging here instead, accounting for extra traces
					//float rayDamage = RANDOM_LONG(16, 128);
					float rayDamage = 32;
					Vector oldRayVel = pHit->pev->velocity;	
					pHit->TakeDamage(owner->pev, owner->pev, rayDamage, DMG_SHOCK);
					pHit->pev->velocity = oldRayVel; // prevent high damage from launching unless we ask for it (unless DMG_LAUNCH)
						
					if (!targets.count(pHit->entindex()))
					{
						targets.insert(pHit->entindex()); // only 1 effect per monstie
						UTIL_ExplosionMsg(tr.vecEndPos, MODEL_INDEX("sprites/doom/bfe2.spr"), 10, 5, 15);
					}
				}
			}
		}
			
	}

	pev->velocity = g_vecZero;
	EMIT_SOUND_DYN(edict(), CHAN_BODY, deathSound, 1.0f, 0.5f, 0, 100);
}
	
bool CDoomProjectile::FireThink()
{
	if (deathTime < gpGlobals->time)
	{
		Remove();
		return false;
	}
	if (fire_state % 2 == 0)
	{
		if (fire_state == 10)
			frameCounter++;
		frameCounter++;
	}
	else if (fire_state % 2 == 1)
		frameCounter--;
			
	fire_state = (fire_state+1) % 11;
		
	CBaseEntity* target = h_aimEnt;
	TraceResult tr;
	Vector followPos = h_followEnt ? h_followEnt.GetEntity()->pev->origin : pev->origin;
	UTIL_TraceLine( target->pev->origin, h_followEnt.GetEntity()->pev->origin, ignore_monsters, NULL, &tr );
	bool targetVisible = tr.flFraction >= 1.0f;
	if (targetVisible)
	{
		g_engfuncs.pfnMakeVectors(target->IsPlayer() ? target->pev->v_angle : target->pev->angles);
		Vector offset = target->IsPlayer() ? Vector(0,0,-35) : Vector(0,0,0);
		Vector newPos = target->pev->origin + gpGlobals->v_forward*16 + offset;
		UTIL_SetOrigin(pev, newPos);
	}

	pev->frame = moveFrameStart + (frameCounter) % ((moveFrameEnd-moveFrameStart) + 1);
	pev->nextthink = gpGlobals->time + 0.05;
	return targetVisible;
}
	
void CDoomProjectile::Think()
{
	if (is_vile_fire) {
		FireThink();
		return;
	}
	frameCounter++;
	
	UTIL_SetOrigin(pev, pev->origin);
		
	if (dead)
	{
		pev->frame = frameCounter;
		if (pev->frame > deathFrameEnd)
		{
			UTIL_Remove(this);
			return;
		}
			
		int flash_size = is_bfg ? 60 : 30;
		int flash_life = 3;
		int flash_decay = 8;
		RGB color = RGB(int(flash_color.x/8), int(flash_color.y/8), int(flash_color.z/8));
		UTIL_DLight(pev->origin, flash_size, color, flash_life, flash_decay);
			
		pev->nextthink = gpGlobals->time + 0.15;
	}
	else
	{
		pev->frame = moveFrameStart + (frameCounter / 3) % ((moveFrameEnd-moveFrameStart) + 1);
			
		int flash_size = is_bfg ? 40 : 20;
		int flash_life = 1;
		int flash_decay = 8;
		RGB color = RGB(flash_color);
		if (is_bfg)
			color = RGB(int(flash_color.x/4), int(flash_color.y/4), int(flash_color.z/4));
		UTIL_DLight(pev->origin, flash_size, color, flash_life, flash_decay);
				
		if (h_followEnt)
		{
			CBaseEntity* followEnt = h_followEnt;
			Vector dir = pev->velocity.Normalize();
			Vector targetDir = ((followEnt->pev->origin + followEnt->pev->view_ofs) - pev->origin).Normalize();
			
			float speed = pev->velocity.Length();	
	
			Vector axis = CrossProduct(dir, targetDir).Normalize();
				
			float dot = DotProduct(targetDir, dir);
			float angle = -acos(dot);
			if (dot == -1)
				angle = M_PI / 2.0f;
			if (dot == 1 || angle != angle)
				angle = 0;
			float maxAngle = 10.0f * M_PI / 180.0f;
			angle = V_max(-maxAngle, V_min(maxAngle, angle));
				
			if (abs(angle) > 0.001f)
			{
				// Apply rotation around arbitrary axis
				vector<float> rotMat = rotationMatrix(axis, angle);
				dir = matMultVector(rotMat, dir).Normalize();
				pev->velocity = dir*speed;
				g_engfuncs.pfnVecToAngles(pev->velocity, pev->angles);	
			}
		}
			
		if (trailSprite)
		{
			if (trailFrame)
				UTIL_ExplosionMsg(pev->origin - pev->velocity.Normalize(), MODEL_INDEX(trailSprite), 14, 10, 15);
			trailFrame = !trailFrame;
		}
			
		pev->nextthink = gpGlobals->time + 0.05;
	}
		
	if (!h_followEnt.GetEntity() && (pev->velocity - lastVelocity).Length() > 1)
	{
		Touch(CBaseEntity::Instance(0));
	}
	lastVelocity = pev->velocity;

	UTIL_MakeVectors(pev->angles);
	forwardDir = gpGlobals->v_forward;
	rightDir = gpGlobals->v_right;
}