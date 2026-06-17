#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "explosive_base.h"
#include "decals.h"

// For Grenade Explosion Sounds
const char* ExplosiveBase::GrenadeExplode[3] = {
	"ins2/wpn/gren/det1.wav",
	"ins2/wpn/gren/det2.wav",
	"ins2/wpn/gren/det3.wav"
};
const char* ExplosiveBase::GrenadeWaterExplode[3] = {
	"ins2/wpn/gren/wdet1.wav",
	"ins2/wpn/gren/wdet2.wav",
	"ins2/wpn/gren/wdet3.wav"
};
// For Grenade Bounce Sounds
const char* ExplosiveBase::GrenadeBounce[4] = {
	"ins2/wpn/gren/hit1.wav",
	"ins2/wpn/gren/hit2.wav",
	"ins2/wpn/gren/hit3.wav",
	"ins2/wpn/gren/hit4.wav"
};
// For Rocket Explosion Sounds
const char* ExplosiveBase::RocketExplode[3] = {
	"ins2/wpn/rckt/det1.wav",
	"ins2/wpn/rckt/det2.wav",
	"ins2/wpn/rckt/det3.wav"
};
const char* ExplosiveBase::RocketWaterExplode[3] = {
	"ins2/wpn/rckt/wdet1.wav",
	"ins2/wpn/rckt/wdet2.wav",
	"ins2/wpn/rckt/wdet3.wav"
};
// For C4s, Shaped Charges
const char* ExplosiveBase::ChargeExplode[3] = {
	"ins2/wpn/chrg/det1.wav",
	"ins2/wpn/chrg/det2.wav",
	"ins2/wpn/chrg/det3.wav"
};
const char* ExplosiveBase::ChargeWaterExplode[3] = {
	"ins2/wpn/chrg/wdet1.wav",
	"ins2/wpn/chrg/wdet2.wav",
	"ins2/wpn/chrg/wdet3.wav"
};
// For C4/TNT Bounce Sounds
const char* ExplosiveBase::C4Bounce[3] = {
	"ins2/wpn/c4/hit1.wav",
	"ins2/wpn/c4/hit2.wav",
	"ins2/wpn/c4/hit3.wav"
};
const char* ExplosiveBase::TNTBounce[3] = {
	"ins2/wpn/tnt/hit1.wav",
	"ins2/wpn/tnt/hit2.wav",
	"ins2/wpn/tnt/hit3.wav"
};

void ExplosiveBase::SelfExplode(CBasePlayer* pPlayer)
{
	CBasePlayerItem* item = pPlayer->GetNamedPlayerItem(STRING(pev->classname));
	float damage = item ? item->pev->dmg : 0;
	entvars_t* ownerVars = pev->owner ? &pev->owner->v : NULL;

	UTIL_ScreenShake(pPlayer->pev->origin, 15.0, 50.0, 1.0, pev->dmg * 2.5);

	ExplodeMsg01(pev->origin, int((damage - 50) * 0.60), 15);

	int iContents = UTIL_PointContents(pev->origin);

	EMIT_SOUND_DYN(edict(), CHAN_AUTO, (iContents == CONTENTS_WATER) ? GrenadeWaterExplode[RANDOM_LONG(0, ARRAY_SZ(GrenadeWaterExplode) - 1)] :
		GrenadeExplode[RANDOM_LONG(0, ARRAY_SZ(GrenadeExplode) - 1)], 1.0f, 0.3, 0, PITCH_NORM);

	RadiusDamage(pPlayer->pev->origin, pPlayer->pev, ownerVars, damage, damage * 2.5f, CLASS_NONE, DMG_BLAST);

	SetThink(&ExplosiveBase::Smoke);
	pev->nextthink = gpGlobals->time + 0.3;

	TraceResult globalTrace = UTIL_GetGlobalTrace();

	UTIL_Sparks(pev->origin);
	UTIL_DecalTrace(&globalTrace, RANDOM_LONG(0, 1) ? DECAL_SCORCH1 : DECAL_SCORCH2);
	if (iContents != CONTENTS_WATER)
	{
		int sparkCount = RANDOM_LONG(1, 3);
		for (int i = 0; i < sparkCount; i++)
			CBaseEntity::Create("spark_shower", pev->origin, globalTrace.vecPlaneNormal, false);
	}
}

void ExplosiveBase::ExplodeMsg01(const Vector& origin, float scale, int framerate)
{
	int iContents = UTIL_PointContents(origin);

	const char* sprite = "sprites/zerogxplode.spr";
	if (iContents == CONTENTS_WATER || iContents == CONTENTS_SLIME || iContents == CONTENTS_LAVA)
		sprite = "sprites/WXplo1.spr";

	UTIL_ExplosionMsg(origin, MODEL_INDEX(sprite), scale, framerate, TE_EXPLFLAG_NOSOUND);
}

void ExplosiveBase::ExplodeMsg02(const Vector& origin, float scale, int framerate)
{
	int iContents = UTIL_PointContents(origin);
	if (!(iContents == CONTENTS_WATER || iContents == CONTENTS_SLIME || iContents == CONTENTS_LAVA)) {
		UTIL_ExplosionMsg(origin, MODEL_INDEX(SPR_EXPLOSION), scale, framerate, TE_EXPLFLAG_NOSOUND);
	}
}

void ExplosiveBase::SmokeMsg(const Vector& origin, float scale, int framerate, const char* spr_path)
{
	UTIL_Smoke(origin, MODEL_INDEX(spr_path), scale, framerate);
}

void ExplosiveBase::Smoke()
{
	int iContents = UTIL_PointContents(pev->origin);
	if (iContents == CONTENTS_WATER || iContents == CONTENTS_SLIME || iContents == CONTENTS_LAVA) {
		UTIL_Bubbles(pev->origin - Vector(64, 64, 64), pev->origin + Vector(64, 64, 64), 100);
	}
	else {
		UTIL_Smoke(pev->origin, MODEL_INDEX("sprites/steam1.spr"), int((pev->dmg - 50) * 0.50), 15);
	}
}