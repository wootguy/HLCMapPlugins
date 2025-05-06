#include "extdll.h"
#include "util.h"
#include "CBaseEntity.h"
#include "CBeam.h"
#include "CSprite.h"
#include "Scheduler.h"

void BeamRemove(EHANDLE h_beam)
{
	UTIL_Remove(h_beam);
}

void WarpballThink(EHANDLE h_entity)
{
	if (!h_entity)
		return;
	EMIT_SOUND(h_entity->edict(), CHAN_ITEM, "debris/beamstart7.wav", 1, ATTN_NORM);
	h_entity->SUB_UseTargets(h_entity, USE_TOGGLE, 0);
}

class CEnvWarpball : public CBaseEntity
{
	string_t m_iszMaster;

	void Precache()
	{
		PRECACHE_MODEL("sprites/lgtning.spr");
		PRECACHE_MODEL("sprites/Fexplo1.spr");
		PRECACHE_MODEL("sprites/XFlare1.spr");

		PRECACHE_SOUND("debris/beamstart2.wav");
		PRECACHE_SOUND("debris/beamstart7.wav");
	}

	void KeyValue(KeyValueData* pkvd)
	{
		if (FStrEq(pkvd->szKeyName, "master"))
		{
			m_iszMaster = ALLOC_STRING(pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
		else
			CBaseEntity::KeyValue(pkvd);
	}

	void Spawn()
	{
		Precache();
	}

	int ObjectCaps()
	{
		return CBaseEntity::ObjectCaps() & ~FCAP_ACROSS_TRANSITION;
	}

	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
	{
		if (m_iszMaster && !UTIL_IsMasterTriggered(m_iszMaster, this))
		{
			return;
		}

		int iTimes = 0;
		int iDrawn = 0;
		TraceResult tr;
		Vector vecDest;
		while (iDrawn < int(pev->frags) && iTimes < int(pev->frags * 3)) // try to draw <frags> beams, but give up after 3x<frags> tries.
		{
			vecDest = pev->health * (Vector(RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1), RANDOM_FLOAT(-1, 1)).Normalize());
			TRACE_LINE(pev->origin, pev->origin + vecDest, ignore_monsters, NULL, &tr);

			if (tr.flFraction != 1.0)
			{
				// we hit something.
				iDrawn++;
				CBeam* pBeam = CBeam::BeamCreate("sprites/lgtning.spr", 200);
				pBeam->PointsInit(pev->origin, tr.vecEndPos);
				pBeam->SetColor(197, 243, 169);
				pBeam->SetNoise(65);
				pBeam->SetBrightness(150);
				pBeam->SetWidth(18);
				EHANDLE h_beam = pBeam->edict();
				g_Scheduler.SetTimeout(BeamRemove, 1.0f, h_beam);
				pBeam->SetScrollRate(35);
			}
			iTimes++;
		}
		EMIT_SOUND(edict(), CHAN_BODY, "debris/beamstart2.wav", 1.0f, ATTN_NORM);
		//EMIT_SOUND( edict(), CHAN_STATIC, "../media/valve.mp3", 1.0f, ATTN_NONE );

		CSprite* pSpr = CSprite::SpriteCreate("sprites/Fexplo1.spr", pev->origin, true);
		pSpr->AnimateAndDie(10);
		pSpr->SetTransparency(kRenderGlow, 77, 210, 130, 255, kRenderFxNoDissipation);

		CSprite* pSpr2 = CSprite::SpriteCreate("sprites/XFlare1.spr", pev->origin, true);
		pSpr2->AnimateAndDie(10);
		pSpr2->SetTransparency(kRenderGlow, 184, 250, 214, 255, kRenderFxNoDissipation);

		g_Scheduler.SetTimeout(WarpballThink, 0.5f, EHANDLE(edict()));
	}
};

LINK_ENTITY_TO_CLASS(env_warpball, CEnvWarpball)
