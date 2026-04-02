#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "weapon_custom.h"
#include "CWeaponCustomEffect.h"
#include "CWeaponCustomSound.h"
#include "Scheduler.h"
#include "te_effects.h"
#include "weapons.h"
#include "CWeaponCustomProjectile.h"
#include "CWeaponCustomShoot.h"

void WeaponCustomProjectile::Spawn()
{
	options = ((CWeaponCustomShoot*)h_shoot_opts.GetEntity())->projectile;

	pev->movetype = options.gravity != 0 ? MOVETYPE_BOUNCE : MOVETYPE_BOUNCEMISSILE;
	pev->solid = SOLID_BBOX;

	SET_MODEL(edict(), STRING(pev->model));

	pev->mins = Vector(-options.size, -options.size, -options.size);
	pev->maxs = Vector(options.size, options.size, options.size);
	pev->angles = pev->angles + options.angles;
	//pev->avelocity = options.avel;
	//pev->friction = 1.0f - options.elasticity;

	pev->frame = 0;
	pev->sequence = 0;
	pev->air_finished = 0; // set to 1 externally when this needs to die
	ResetSequenceInfo();

	SetThink(&WeaponCustomProjectile::MoveThink);
	pev->nextthink = gpGlobals->time + thinkDelay;

	move_snd_playing = options.move_snd.play(this, CHAN_BODY);
}

void WeaponCustomProjectile::MoveThink()
{
	float nextThink = gpGlobals->time + thinkDelay;
	CWeaponCustomShoot* shoot_opts = (CWeaponCustomShoot*)h_shoot_opts.GetEntity();

	if (pev->air_finished > 0 || !shoot_opts)
	{
		uninstall_steam_and_kill_yourself();
		return;
	}

	if (attached && target)
	{
		CBaseEntity* tar = target;

		// rotate position around target
		Vector newOri = attachStartOri + (tar->pev->origin - targetStartOri);
		newOri = rotatePoint(newOri - tar->pev->origin, -tar->pev->angles) + tar->pev->origin;

		// rotate orientation around target
		Vector newDir = rotatePoint(attachStartDir, -tar->pev->angles);
		g_engfuncs.pfnVecToAngles(newDir, pev->angles);

		pev->origin = newOri;

		// prevent sudden jerking due to movement lagging behind the Touch() event
		attachTime++;
		if (attachTime > 2) {
			pev->velocity = Vector(0, 0, 0);
			pev->movetype = MOVETYPE_FLY;
		}

		if (shoot_opts->hook_type != HOOK_DISABLED)
		{
			CBaseEntity* owner = CBaseEntity::Instance(pev->owner);

			Vector dir = (pev->origin - owner->pev->origin).Normalize();
			Vector repelDir = dir;
			repelDir.z = 0;

			//if (DotProduct(dir, owner->pev->velocity.Normalize()) < 0)
			//	owner->pev->velocity = -owner->pev->velocity;

			float x = tar->pev->maxs.x - tar->pev->mins.x;
			float y = tar->pev->maxs.y - tar->pev->mins.y;
			float z = tar->pev->maxs.z - tar->pev->mins.z;
			float size = x * y * z;
			float playerSize = 32 * 32 * 72;
			//println("SIZE: " + x + "x" + y + "x" + z + " " + size);

			bool pullMode = shoot_opts->hook_pull_mode == HOOK_MODE_PULL ||
				shoot_opts->hook_pull_mode == HOOK_MODE_PULL_LEAST_WEIGHT;
			bool pullUser = shoot_opts->hook_pull_mode == HOOK_MODE_PULL_LEAST_WEIGHT && playerSize <= size;
			pullUser = pullUser || shoot_opts->hook_pull_mode == HOOK_MODE_PULL || tar->IsBSPModel();

			if (pullMode)
			{
				if (pullUser)
					owner->pev->velocity = owner->pev->velocity + dir * shoot_opts->hook_force;
				else
					tar->pev->velocity = tar->pev->velocity - dir * shoot_opts->hook_force * 0.5;
			}
			else
				owner->pev->velocity = owner->pev->velocity + repelDir * shoot_opts->hook_force;

			if (pullUser)
			{
				if (owner->pev->velocity.Length() > shoot_opts->hook_max_speed)
					owner->pev->velocity = resizeVector(owner->pev->velocity, shoot_opts->hook_max_speed);

				if (pullMode && owner->pev->flags & FL_ONGROUND != 0 && dir.z > 0)
				{
					// The player is going to be stubborn && glue itself to the ground.
					// Make sure that forcing the player upward won't jam it into something solid
					TraceResult tr;
					Vector vecSrc = owner->pev->origin;
					Vector vecEnd = owner->pev->origin + Vector(0, 0, 2);
					UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, human_hull, owner->edict(), &tr);
					if (tr.flFraction >= 1.0)
						owner->pev->origin.z += 2; // cool, we're all clear
				}
			}

			nextThink = gpGlobals->time; // don't let gravity overpower the pull force too easily
		}

		if (!tar->IsBSPModel() && !tar->IsAlive())
		{
			attached = false;
			target = NULL;
			if (shoot_opts->hook_type != HOOK_DISABLED)
			{
				uninstall_steam_and_kill_yourself();
				return;
			}
			pev->movetype = options.gravity != 0 ? MOVETYPE_BOUNCE : MOVETYPE_BOUNCEMISSILE;
		}
	}
	else
	{
		if (attached)
		{
			attached = false;
			if (shoot_opts->hook_type != HOOK_DISABLED)
			{
				uninstall_steam_and_kill_yourself();
				return;
			}
			pev->movetype = options.gravity != 0 ? MOVETYPE_BOUNCE : MOVETYPE_BOUNCEMISSILE;
		}
		if (shoot_opts->pev->spawnflags & FL_SHOOT_PROJ_NO_ORIENT == 0)
			g_engfuncs.pfnVecToAngles(pev->velocity, pev->angles);
	}

	if (move_snd_playing && pev->velocity.Length() == 0)
		options.move_snd.stop(this, CHAN_BODY);

	bool noBubbles = shoot_opts->pev->spawnflags & FL_SHOOT_NO_BUBBLES != 0;
	bool inWater = g_engfuncs.pfnPointContents(pev->origin) == CONTENTS_WATER;
	if (!attached && !noBubbles && inWater)
	{
		if (nextBubbleTime < gpGlobals->time)
		{
			Vector pos = pev->origin;
			//float waterLevel = UTIL_WaterLevel(pos, pos.z, pos.z + 1024) - pos.z;
			//UTIL_BubbleTrail(pos, pos, "sprites/bubble.spr", waterLevel, 1, 16.0f);
			UTIL_BubbleTrail(pos, pos, 1);
			nextBubbleTime = gpGlobals->time + bubbleDelay;
		}
	}

	if (inWater && options.water_friction != 0)
	{
		float speed = pev->velocity.Length();
		if (speed > 0)
			pev->velocity = resizeVector(pev->velocity, speed - speed * options.water_friction);
	}
	else if (!inWater && options.air_friction != 0)
	{
		float speed = pev->velocity.Length();
		if (speed > 0)
			pev->velocity = resizeVector(pev->velocity, speed - speed * options.air_friction);
	}

	CWeaponCustomEffect* effect4 = (CWeaponCustomEffect*)shoot_opts->effect4.GetEntity();
	if (effect4 && effect4->valid && nextTrailEffectTime < gpGlobals->time)
	{
		nextTrailEffectTime = gpGlobals->time + options.trail_effect_freq;
		CBaseEntity* owner = CBaseEntity::Instance(pev->owner);
		EHANDLE howner = EHANDLE(owner->edict());
		custom_effect(pev->origin, shoot_opts->effect4, EHANDLE(edict()), howner, howner, pev->velocity.Normalize(), shoot_opts->friendly_fire ? 1 : 0);
	}

	if (weaponPickup)
	{
		if (pev->velocity.Length() < 128)
		{
			if (!attached && pev->flags & FL_ONGROUND != 0)
			{
				pev->angles.x = 0;
				pev->angles.z = 0;
			}

			CBaseEntity* ent = NULL;
			do {
				ent = UTIL_FindEntityInSphere(ent, pev->origin, pickupRadius);
				if (ent && ent->IsPlayer())
				{
					CBasePlayer* plr = (CBasePlayer*)(ent);
					//plr->SetItemPickupTimes(0);
					EALERT(at_error, "Player set item pickup times not implemented\n");
					if (plr->HasNamedPlayerItem(STRING(pickup_classname)))
					{
						// play the pickup sound even if they already have the weapon
						EMIT_SOUND_DYN(plr->edict(), CHAN_ITEM, "items/gunpickup2.wav", 1.0,
							ATTN_NORM, 0, 100);
					}
					else
						plr->GiveNamedItem(STRING(pickup_classname));
					uninstall_steam_and_kill_yourself();
				}
			} while (ent);
		}
	}

	pev->nextthink = nextThink;
}

void WeaponCustomProjectile::uninstall_steam_and_kill_yourself()
{
	if (move_snd_playing)
		options.move_snd.stop(this, CHAN_BODY);
	UTIL_Remove(this);
	if (spriteAttachment)
		UTIL_Remove(spriteAttachment);
}

void WeaponCustomProjectile::DamageTarget(CBaseEntity* ent, bool friendlyFire)
{
	CWeaponCustomShoot* shoot_opts = (CWeaponCustomShoot*)h_shoot_opts.GetEntity();

	if (ent == NULL || ent->entindex() == 0 || !shoot_opts || shoot_opts->shoot_type == SHOOT_MELEE)
		return;
	CBaseEntity* owner = CBaseEntity::Instance(pev->owner);

	// damage done before hitgroup multipliers
	float baseDamage = shoot_opts->damage;

	if (baseDamage == 0)
		return;

	baseDamage = applyDamageModifiers(baseDamage, ent, owner, shoot_opts);

	TraceResult tr;
	Vector vecSrc = pev->origin;
	Vector vecAiming = pev->velocity.Normalize();
	Vector vecEnd = vecSrc + vecAiming * pev->velocity.Length() * 2;
	UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, edict(), &tr);
	CBaseEntity* pHit = tr.pHit ? CBaseEntity::Instance(tr.pHit) : NULL;
	//te_beampoints(vecSrc, tr.vecEndPos);
	if (tr.flFraction >= 1.0 || pHit->entindex() != ent->entindex())
	{
		// This does a trace in the form of a box so there is a much higher chance of hitting something
		// From crowbar.cpp in the hlsdk:
		UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, edict(), &tr);
		if (tr.flFraction < 1.0)
		{
			// Calculate the point of intersection of the line (or hull) && the object we hit
			// This is && approximation of the "best" intersection
			pHit = CBaseEntity::Instance(tr.pHit);
			if (pHit == NULL || pHit->IsBSPModel())
				FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, edict());
			vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
			vecAiming = (vecEnd - vecSrc).Normalize();
		}
	}

	Vector oldVel = ent->pev->velocity;
	int dmgType = shoot_opts->damageType(DMG_CLUB);

	ClearMultiDamage(); // fixes TraceAttack() crash for some reason
	ent->TraceAttack(owner->pev, baseDamage, vecAiming, &tr, dmgType);

	if (friendlyFire)
	{
		// set both classes in case this a pvp map where classes are always changing
		int oldClass1 = owner->Classify();
		int oldClass2 = ent->Classify();
		owner->SetClassification(CLASS_PLAYER);
		ent->SetClassification(CLASS_ALIEN_MILITARY);
		ApplyMultiDamage(owner->pev, owner->pev);
		owner->SetClassification(oldClass1);
		ent->SetClassification(oldClass2);
	}
	else
		ApplyMultiDamage(owner->pev, owner->pev);

	if ((dmgType & DMG_LAUNCH) == 0) // prevent high damage from launching unless we ask for it
		ent->pev->velocity = oldVel;

	WeaponSound* impact_snd;
	if ((ent->IsMonster() || ent->IsPlayer()) && !ent->IsMachine())
		impact_snd = shoot_opts->getRandomMeleeFleshSound();
	else
		impact_snd = shoot_opts->getRandomMeleeHitSound();

	if (impact_snd)
		impact_snd->play(this, CHAN_WEAPON);
}

void WeaponCustomProjectile::ConvertToWeapon()
{
	if (options.type == PROJECTILE_WEAPON)
	{
		pev->angles.z = 0;
		weaponPickup = true;
		if (move_snd_playing)
			options.move_snd.stop(this, CHAN_BODY);
	}
}

bool WeaponCustomProjectile::isValidHookSurface(CBaseEntity* pOther)
{
	CWeaponCustomShoot* shoot_opts = (CWeaponCustomShoot*)h_shoot_opts.GetEntity();
	if (!shoot_opts)
		return false;

	if (pOther->IsBSPModel())
	{
		if (shoot_opts->hook_targets == HOOK_MONSTERS_ONLY)
			return false;

		if (shoot_opts->hook_texture_filter)
		{
			DecalTarget dt = getProjectileDecalTarget(this, Vector(0, 0, 0), 1);
			string hitTex = toLowerCase(dt.texture);
			string matchTex = toLowerCase(STRING(shoot_opts->hook_texture_filter));
			if (hitTex.find(matchTex) != 0)
				return false;
		}
	}

	if (shoot_opts->hook_targets == HOOK_WORLD_ONLY || !pOther->IsAlive())
		return false;

	return true;
}

void WeaponCustomProjectile::Touch(CBaseEntity* pOther)
{
	CWeaponCustomShoot* shoot_opts = (CWeaponCustomShoot*)h_shoot_opts.GetEntity();

	if (attached || !shoot_opts)
		return;

	int event = PROJ_ACT_BOUNCE;
	CWeaponCustomEffect* effect = (CWeaponCustomEffect*)shoot_opts->effect1.GetEntity();

	if (pOther->IsBSPModel())
		event = options.world_event;
	else
	{
		effect = (CWeaponCustomEffect*)shoot_opts->effect2.GetEntity();
		event = options.monster_event;
	}

	pev->velocity = pev->velocity * options.elasticity;

	if (weaponPickup)
	{
		// no more special effects after the first impact (pretend we're a weaponbox)
		if (effect) {
			WeaponSound* rico_snd = effect->getRandomSound();
			if (rico_snd)
				rico_snd->play(pev->origin, CHAN_STATIC);
		}
		pev->avelocity.x *= -0.9;
		return;
	}

	DamageTarget(pOther, shoot_opts->friendly_fire);
	knockBack(pOther, pev->velocity.Normalize() * shoot_opts->knockback);

	// don't spam bounce sounds when rolling on ground
	if (event != PROJ_ACT_BOUNCE || nextBounceEffect < gpGlobals->time)
	{
		nextBounceEffect = gpGlobals->time + options.bounce_effect_delay;
		CBaseEntity* owner = CBaseEntity::Instance(pev->owner);
		EHANDLE howner = owner->edict();
		EHANDLE htarget = pOther->edict();
		if (effect)
			custom_effect(pev->origin, effect->edict(), EHANDLE(edict()), htarget, howner, Vector(0, 0, 0), shoot_opts->friendly_fire ? 1 : 0);
	}

	ConvertToWeapon();

	switch (event)
	{
	case PROJ_ACT_IMPACT:
		uninstall_steam_and_kill_yourself();
		return;
	case PROJ_ACT_ATTACH:
		if (shoot_opts->hook_type != HOOK_DISABLED && !isValidHookSurface(pOther))
		{
			uninstall_steam_and_kill_yourself();
			return;
		}

		target = pOther;
		attachStartOri = unwindPoint(pev->origin - pOther->pev->origin, -pOther->pev->angles) + pOther->pev->origin;
		attachStartDir = unwindPoint(pev->velocity.Normalize(), -pOther->pev->angles);
		targetStartOri = pOther->pev->origin;
		pev->solid = SOLID_NOT;
		pev->velocity = Vector(0, 0, 0);
		pev->avelocity = Vector(0, 0, 0);
		attached = true;

		// attach to center of monster
		if (shoot_opts->hook_type != HOOK_DISABLED && pOther->IsMonster())
		{
			CBaseMonster* mon = pOther->MyMonsterPointer();
			float height = mon->pev->maxs.z - mon->pev->mins.z;
			attachStartOri = mon->pev->origin + Vector(0, 0, height * 0.5f);
		}
		return;
	}
}

LINK_ENTITY_TO_CLASS(custom_projectile, WeaponCustomProjectile)