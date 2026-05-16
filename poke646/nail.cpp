#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

const float NAILGUN_MOD_DAMAGE = 15.0;

class CNail : public CProjectileCustom {
	void Spawn() {
		CProjectileCustom::Spawn();

		Vector vecDir = pev->velocity.Normalize();

		pev->velocity = vecDir * 4000;
		pev->avelocity.z = 10;
		ParametricInterpolation(0.1f);

		UTIL_BeamFollow(entindex(), MODEL_INDEX("sprites/laserbeam.spr"), 1, 1, RGB(64, 64, 64));
	}

	virtual const char* GetDeathNoticeWeapon() { 
		CBaseEntity* owner = CBaseEntity::Instance(pev->owner);
		return owner ? owner->GetDeathNoticeWeapon() : "weapon_9mmhandgun";
	}

	void CustomTouch(CBaseEntity* pOther) override {
		if (pOther->pev->takedamage)
		{
			TraceResult tr = UTIL_GetGlobalTrace();
			entvars_t* pevOwner;

			pevOwner = VARS(pev->owner);

			ClearMultiDamage();
			pOther->TraceAttack(pevOwner, NAILGUN_MOD_DAMAGE, pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);
			ApplyMultiDamage(pev, pevOwner);

			pev->velocity = Vector(0, 0, 0);

			const char* snd = RANDOM_LONG(0, 1) ? "weapons/xbow_hitbod1.wav" : "weapons/xbow_hitbod2.wav";
			EMIT_SOUND(ENT(pev), CHAN_BODY, snd, 1, ATTN_NORM);
		}
		else
		{
			SetThink(&CBaseEntity::SUB_Remove);
			pev->nextthink = gpGlobals->time;// this will get changed below if the bolt is allowed to stick in what it hit.

			if (FClassnameIs(pOther->pev, "worldspawn"))
			{
				// if what we hit is static architecture, can stay around for a while.
				Vector vecDir = pev->velocity.Normalize();
				UTIL_SetOrigin(pev, pev->origin - vecDir * 8);
				pev->angles = UTIL_VecToAngles(vecDir);
				pev->solid = SOLID_NOT;
				pev->movetype = MOVETYPE_FLY;
				pev->velocity = Vector(0, 0, 0);
				pev->avelocity.z = 0;
				pev->angles.z = RANDOM_LONG(0, 360);
				pev->nextthink = gpGlobals->time + 3.0;

				pev->starttime = 0;
				pev->impacttime = 0;
			}

			if (UTIL_PointContents(pev->origin) != CONTENTS_WATER) {
				if (RANDOM_LONG(0,2) == 0)
					UTIL_Sparks(pev->origin);
			}

			return;
		}

		UTIL_Remove(this);
	}

	void CustomMove(void) override
	{
		pev->nextthink = gpGlobals->time + 0.1;
		ParametricInterpolation(0.1f);

		if (pev->waterlevel == 0)
			return;

		UTIL_BubbleTrail(pev->origin - pev->velocity * 0.1, pev->origin, 1);
	}
};

LINK_ENTITY_TO_CLASS(nail, CNail)