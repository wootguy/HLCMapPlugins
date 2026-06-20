#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "CSoundEnt.h"
#include "decals.h"
#include "explosive_base.h"

class CIns2GL : public ExplosiveBase
{
	bool bRcktExplsions;

	void Spawn() override {
		ExplosiveBase::Spawn();
	}

	void Configure(CBasePlayer* attackr, CWeaponCustom* weapon, WepEvt& evt) override {
		int iContents = UTIL_PointContents(pev->origin);
		if (iContents == CONTENTS_WATER || iContents == CONTENTS_SLIME || iContents == CONTENTS_LAVA)
			pev->velocity = pev->velocity * 0.5f;
		pev->dmg = damage;
		pev->sequence = 1;
		pev->angles.x -= 90;
	}

	void Precache() override
	{
		ExplosiveBase::Precache();
		PRECACHE_MODEL("sprites/smoke.spr");
		PRECACHE_MODEL("sprites/zerogxplode.spr");
		PRECACHE_MODEL(SPR_EXPLOSION);
		PRECACHE_MODEL("sprites/WXplo1.spr");
		PRECACHE_MODEL("sprites/steam1.spr");

		PRECACHE_SOUND_ARRAY(GrenadeExplode)
		PRECACHE_SOUND_ARRAY(GrenadeWaterExplode)
		PRECACHE_SOUND_ARRAY(RocketExplode)
		PRECACHE_SOUND_ARRAY(RocketWaterExplode)
	}

	void DamageTarget(CBaseEntity* ent) override {}

	void CustomMove() override {
		CProjectileCustom::CustomMove();
		pev->angles.x -= 90;
	}

	void Impact(CBaseEntity* pOther) override
	{
		pev->model = string_t();
		pev->solid = SOLID_NOT;
		pev->takedamage = DAMAGE_NO;
		int iContents = UTIL_PointContents(pev->origin);

		UTIL_ScreenShake(pev->origin, 15.0, 50.0, 1.0, pev->dmg * 3.0);

		TraceResult tr;

		Vector vecSpot = pev->origin - pev->velocity.Normalize() * 32;
		Vector vecEnd = pev->origin + pev->velocity.Normalize() * 64;

		entvars_t* pevOwner;
		if (pev->owner)
			pevOwner = &(pev->owner->v);
		else
			pevOwner = pev;

		UTIL_TraceLine(vecSpot, vecEnd, ignore_monsters, edict(), &tr);

		//Move it out of the wall a bit
		if (tr.flFraction != 1.0)
		{
			pev->origin = tr.vecEndPos + (tr.vecPlaneNormal * (pev->dmg - 24) * 0.55);
		}

		RadiusDamage(pev->origin, pev, pevOwner, pev->dmg, pev->dmg * 2, CLASS_NONE, DMG_BLAST);

		ExplodeMsg01(pev->origin, (!bRcktExplsions) ? (pev->dmg - 30) * 0.32 : (pev->dmg - 50) * 0.315, 15);
		ExplodeMsg02(pev->origin, (!bRcktExplsions) ? (pev->dmg - 30) * 0.30 : (pev->dmg - 50) * 0.30, 30);

		//Kill the beam following the grenade
		UTIL_KillBeam(entindex(), MSG_BROADCAST);
		
		CSoundEnt::InsertSound(bits_SOUND_COMBAT, pev->origin, NORMAL_EXPLOSION_VOLUME, 1.5, this);

		UTIL_DecalTrace(&tr, RANDOM_LONG(0, 1) ? DECAL_SCORCH1 : DECAL_SCORCH2);

		if (!bRcktExplsions) {
			EMIT_SOUND_DYN(edict(), CHAN_AUTO, (iContents == CONTENTS_WATER) ?
				RANDOM_SOUND_ARRAY(GrenadeWaterExplode) :
				RANDOM_SOUND_ARRAY(GrenadeExplode), 1.0f, 0.3, 0, PITCH_NORM);
		}
		else {
			EMIT_SOUND_DYN(edict(), CHAN_AUTO, (iContents == CONTENTS_WATER) ?
				RANDOM_SOUND_ARRAY(RocketWaterExplode) :
				RANDOM_SOUND_ARRAY(RocketExplode), 1.0f, 0.3, 0, PITCH_NORM);
		}

		//Direct hit does additional damage
		if (pOther->pev->takedamage != DAMAGE_NO)
		{
			ClearMultiDamage();
			pOther->TraceAttack(pevOwner, pev->dmg / 1.5, gpGlobals->v_forward, &tr, DMG_CLUB);
			ApplyMultiDamage(pev, pevOwner);
		}

		UTIL_Sparks(pev->origin);

		if (iContents != CONTENTS_WATER)
		{
			int sparkCount = RANDOM_LONG(1, 3);
			for (int i = 0; i < sparkCount; i++)
				CBaseEntity::Create("spark_shower", pev->origin, tr.vecPlaneNormal, false);
		}

		UTIL_WaterSplash(pev->origin, true, true);

		pev->effects |= EF_NODRAW;
		SetThink(&CIns2GL::Smoke);
		pev->velocity = g_vecZero;
		pev->nextthink = gpGlobals->time + 0.3;
	}

	void Smoke()
	{
		int iContents = UTIL_PointContents(pev->origin);
		if (iContents == CONTENTS_WATER || iContents == CONTENTS_SLIME || iContents == CONTENTS_LAVA)
		{
			UTIL_Bubbles(pev->origin - Vector(70, 70, 70), pev->origin + Vector(70, 70, 70), 150);
		}
		else
		{
			SmokeMsg(pev->origin, (bRcktExplsions == false) ? (pev->dmg - 30) * 0.80 : (pev->dmg - 50) * 0.50, 15);
		}

		UTIL_Remove(this);
	}
};

LINK_ENTITY_TO_CLASS(proj_ins2gl, CIns2GL)