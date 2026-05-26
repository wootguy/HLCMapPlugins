#include "extdll.h"
#include "util.h"
#include "CProjectileCustom.h"
#include "te_effects.h"
#include "CBeam.h"
#include "customentity.h"
#include "weapons.h"

const char* g_impactSound = "buttons/latchunlocked1.wav";
const char* g_impactSoundBig = "weapons/mortarhit.wav";
const char* g_beamSpr = "sprites/plasma.spr";

class CPlasmaBall : public CProjectileCustom {
public:
	float m_nextFlare;
	int m_flareSpr;
	float m_impactTime;
	Vector m_impactDir;

	void Precache() override {
		CProjectileCustom::Precache();

		m_flareSpr = PRECACHE_MODEL("sprites/shl/weapons/plasmasmall.spr");
		PRECACHE_SOUND(g_impactSound);
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_egon"; }

	void Impact(CBaseEntity* pOther) override {
		m_impactTime = gpGlobals->time;
		m_impactDir = pev->velocity.Normalize() * -1;
		pev->movetype = MOVETYPE_NONE;
		pev->solid = SOLID_NOT;
		UTIL_GlowSprite(pev->origin, pev->modelindex, 10, 10, 255);
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, g_impactSound, 1.0f, ATTN_NORM, 0, RANDOM_LONG(90, 100));
	}

	void CustomMove(void) override
	{
		CProjectileCustom::CustomMove();

		if (m_impactTime) {
			pev->renderamt -= 10;
			pev->scale += 0.07f;
			UTIL_SetOrigin(pev, pev->origin + m_impactDir * thinkDelay * 800);
			m_impactDir = m_impactDir * 0.5f;

			if (pev->renderamt < 0) {
				pev->renderamt = 0;
				UTIL_Remove(this);
			}

			return;
		}
		
		// original mod is making shade-out beams every tick but that's expensive and the trail
		// looks close enough.
		
		Flare();
	}

	void Flare() {
		if (m_nextFlare < gpGlobals->time) {
			if (m_nextFlare)
				UTIL_Spray(pev->origin, Vector(0, 0, 1), m_flareSpr, 1, 100, 100, 5);
			m_nextFlare = gpGlobals->time + RANDOM_FLOAT(0.05f, 0.3f);
		}
	}
};


class CPlasmaBallBig : public CPlasmaBall {
public:
	void Precache() override {
		CPlasmaBall::Precache();
		PRECACHE_SOUND(g_impactSoundBig);
		PRECACHE_MODEL(g_beamSpr);
	}

	void Impact(CBaseEntity* pOther) override {
		if (!m_impactTime) {
			UTIL_GlowSprite(pev->origin, pev->modelindex, 100, 10, 120);
			EMIT_SOUND_DYN(edict(), CHAN_WEAPON, g_impactSoundBig, 1.0f, ATTN_NORM, 0, RANDOM_LONG(90, 100));
			pev->gravity = 0;
		}
		if (pev->flags & FL_ONGROUND)
			pev->velocity = g_vecZero;
		m_impactTime = gpGlobals->time;
	}

	void CustomMove(void) override
	{
		CProjectileCustom::CustomMove();

		for (int i = 0; i < 3 && pev->renderamt >= 20; i++) {
			Vector dir = UTIL_RandomPointOnSphere() * 128;
			CBeam* beam = CBeam::BeamCreate(g_beamSpr, 100);
			beam->PointEntInit(pev->origin + dir, entindex());
			beam->SetColor(250, 200, 160);
			beam->SetScrollRate(100);
			beam->SetFlags(BEAM_FSHADEIN);
			beam->LiveForTime(0.1f);
		}

		if (m_impactTime) {
			pev->renderamt -= 5;
			pev->scale += 0.07f;

			Vector dir = UTIL_RandomPointOnSphere() * RANDOM_FLOAT(3, 5);
			dir.z = fabs(dir.z);

			UTIL_Spray(pev->origin, dir, m_flareSpr, 6, 100, 10, 5);

			if (pev->renderamt < 0) {
				pev->renderamt = 0;
				UTIL_Remove(this);
			}

			CBaseEntity* owner = Instance(pev->owner);
			RadiusDamage(pev->origin, pev, owner ? owner->pev : NULL, 2, 256, CLASS_NONE, DMG_ENERGYBEAM);

			return;
		}

		Flare();
	}
};

LINK_ENTITY_TO_CLASS(plasmaball, CPlasmaBall)
LINK_ENTITY_TO_CLASS(plasmaballbig, CPlasmaBallBig)