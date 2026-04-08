#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "CProjectileCustom.h"
#include "CWeaponCustomEffect.h"
#include "CWeaponCustomShoot.h"
#include "CWeaponCustomConfig.h"
#include "te_effects.h"
#include "skill.h"
#include "weapon_custom.h"

class CCustomProjectilePlugin : public CProjectileCustom {
public:
	EHANDLE h_shoot_opts;
	bool weaponPickup;
	float pickupRadius;
	string_t pickup_classname;
	float nextTrailEffectTime;
	float nextBounceEffect;
	int proj_type;

	void Configure(CBasePlayer* attacker, CWeaponCustom* weapon, WepEvt& evt) override {
		int attackIdx = clamp(weapon->GetAttackIdx(evt), 0, 3);

		EHANDLE h_settings = *custom_weapons.get(STRING(weapon->pev->classname));

		CWeaponCustomConfig* settings = (CWeaponCustomConfig*)h_settings.GetEntity();
		if (!settings)
			return;

		if (attackIdx == 3) {
			h_shoot_opts = settings->get_alt_shoot_settings(0);
		}
		else {
			h_shoot_opts = settings->get_shoot_settings(attackIdx);
		}
	}

	void Spawn() {
		pickupRadius = 64;
		CProjectileCustom::Spawn();

		CWeaponCustomShoot* shoot_opts = (CWeaponCustomShoot*)h_shoot_opts.GetEntity();
		if (!shoot_opts)
			return;

		proj_type = shoot_opts->projectile.type;
	}

	void ConvertToWeapon() {
		if (proj_type == PROJECTILE_WEAPON) {
			pev->angles.z = 0;
			weaponPickup = true;
			StopMoveSound();
		}
	}

	void CustomMove() override {
		CWeaponCustomShoot* shoot_opts = (CWeaponCustomShoot*)h_shoot_opts.GetEntity();
		if (!shoot_opts)
			return;

		CProjectileCustom::CustomMove();

		if (attached && attachTarget && shoot_opts->hook_type != HOOK_DISABLED)
		{
			CBaseEntity* tar = attachTarget;
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

				if (pullMode && (owner->pev->flags & FL_ONGROUND) != 0 && dir.z > 0)
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

			pev->nextthink = gpGlobals->time; // don't let gravity overpower the pull force too easily
		}

		if (nextTrailEffectTime < gpGlobals->time) {
			nextTrailEffectTime = gpGlobals->time + shoot_opts->projectile.trail_effect_freq;

			CWeaponCustomEffect* effect4 = (CWeaponCustomEffect*)shoot_opts->effect4.GetEntity();
			if (effect4 && effect4->valid)
			{
				nextTrailEffectTime = gpGlobals->time + shoot_opts->projectile.trail_effect_freq;
				CBaseEntity* owner = CBaseEntity::Instance(pev->owner);
				EHANDLE howner = EHANDLE(owner->edict());
				custom_effect(pev->origin, shoot_opts->effect4, EHANDLE(edict()), howner, howner, pev->velocity.Normalize(), shoot_opts->friendly_fire ? 1 : 0);
			}
		}

		if (weaponPickup) {
			if (pev->velocity.Length() < 128) {
				if (!attached && (pev->flags & FL_ONGROUND) != 0) {
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
						Remove();
					}
				} while (ent);
			}
		}
	}

	void Impact(CBaseEntity* ent) override {
		CProjectileCustom::Impact(ent);

		CWeaponCustomShoot* shoot_opts = (CWeaponCustomShoot*)h_shoot_opts.GetEntity();
		if (!shoot_opts)
			return;

		WeaponSound* impact_snd;
		if ((ent->IsMonster() || ent->IsPlayer()) && !ent->IsMachine())
			impact_snd = shoot_opts->getRandomMeleeFleshSound();
		else
			impact_snd = shoot_opts->getRandomMeleeHitSound();

		if (impact_snd)
			impact_snd->play(this, CHAN_WEAPON);
	}

	bool isValidHookSurface(CBaseEntity* pOther) {
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

	void CustomTouch(CBaseEntity* pOther) override {
		CProjectileCustom::CustomTouch(pOther);

		CWeaponCustomShoot* shoot_opts = (CWeaponCustomShoot*)h_shoot_opts.GetEntity();

		if (!shoot_opts)
			return;

		int event = getImpactAction(pOther);
		CWeaponCustomEffect* effect = (CWeaponCustomEffect*)shoot_opts->effect1.GetEntity();

		if (!pOther->IsBSPModel()) {
			effect = (CWeaponCustomEffect*)shoot_opts->effect2.GetEntity();
		}

		pev->velocity = pev->velocity * shoot_opts->projectile.elasticity;

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
		
		knockBack(pOther, pev->velocity.Normalize() * shoot_opts->knockback);

		// don't spam bounce sounds when rolling on ground
		if (event != WC_PROJ_ACT_BOUNCE || nextBounceEffect < gpGlobals->time)
		{
			nextBounceEffect = gpGlobals->time + shoot_opts->projectile.bounce_effect_delay;
			CBaseEntity* owner = CBaseEntity::Instance(pev->owner);
			EHANDLE howner = owner->edict();
			EHANDLE htarget = pOther->edict();
			if (effect)
				custom_effect(pev->origin, effect->edict(), EHANDLE(edict()), htarget, howner, Vector(0, 0, 0), shoot_opts->friendly_fire ? 1 : 0);
		}

		ConvertToWeapon();
	}

	void Attach(CBaseEntity* pOther) override {
		CWeaponCustomShoot* shoot_opts = (CWeaponCustomShoot*)h_shoot_opts.GetEntity();
		if (!shoot_opts)
			return;

		if (shoot_opts->hook_type != HOOK_DISABLED && !isValidHookSurface(pOther))
		{
			Remove();
			return;
		}

		CProjectileCustom::Attach(pOther);

		// attach to center of monster
		if (shoot_opts->hook_type != HOOK_DISABLED && pOther->IsMonster())
		{
			CBaseMonster* mon = pOther->MyMonsterPointer();
			float height = mon->pev->maxs.z - mon->pev->mins.z;
			attachStartOri = mon->pev->origin + Vector(0, 0, height * 0.5f);
		}
	}

	void Die() override {
		CWeaponCustomShoot* shoot_opts = (CWeaponCustomShoot*)h_shoot_opts.GetEntity();
		if (!shoot_opts)
			return;

		CBaseEntity* owner = CBaseEntity::Instance(pev->owner);
		EHANDLE howner = owner->edict();
		EHANDLE target;
		custom_effect(pev->origin, shoot_opts->effect3, EHANDLE(edict()), target, howner,
			Vector(0, 0, 0), shoot_opts->friendly_fire ? 1 : 0);
	}
};

LINK_ENTITY_TO_CLASS(custom_projectile_plugin, CCustomProjectilePlugin)