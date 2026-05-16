#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

const int CMLWBR_MOD_PROJ_SPEED = 2000;
const int CMLWBR_MOD_PROJ_SPEED_UNDERWATER = 1000;

class CCmlwbr : public CWeaponCustom {

	float m_lastText;

	void ItemPostFrame() override {
		CWeaponCustom::ItemPostFrame();

		CBasePlayer* m_pPlayer = GetPlayer();
		if (!m_pPlayer)
			return;

		if (gpGlobals->time - m_lastText < 0.05f) {
			return;
		}

		m_lastText = gpGlobals->time;

		if (m_pPlayer->m_iFOV) {
			hudconparms_t params;
			memset(&params, 0, sizeof(hudconparms_t));
			params.r = 255;
			params.xEm = 20;
			params.yEm = -2;
			params.yPercent = 0.5f;
			params.xPercent = 0.5f;
			params.holdTime = 100;
			params.id = 1;

			TraceResult tr;
			Vector vecSrc = m_pPlayer->GetGunPosition();
			Vector vecEnd = vecSrc + m_pPlayer->GetLookDirection() * 8192;
			UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, m_pPlayer->edict(), &tr);
			float dist = (tr.vecEndPos - vecSrc).Length() * 0.025f;

			const char* msg = UTIL_VarArgs("%.2f m", dist);

			UTIL_ClientHudConPrint(m_pPlayer, params, msg, false);
		}
	}
};

class CCmlwbrBolt : public CProjectileCustom {
public:
	void Spawn() {
		CProjectileCustom::Spawn();

		Vector vecDir = pev->velocity.Normalize();

		pev->velocity = vecDir * (pev->waterlevel == 3 ?
			CMLWBR_MOD_PROJ_SPEED_UNDERWATER : CMLWBR_MOD_PROJ_SPEED);
		pev->avelocity.z = 10;
		ParametricInterpolation(0.1f);
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_crossbow"; }

	void CustomTouch(CBaseEntity* pOther) override {
		if (pOther->pev->takedamage)
		{
			TraceResult tr = UTIL_GetGlobalTrace();
			entvars_t* pevOwner;

			pevOwner = VARS(pev->owner);

			ClearMultiDamage();
			pOther->TraceAttack(pevOwner, GetDamage(gSkillData.sk_plr_xbow_bolt_monster), pev->velocity.Normalize(), &tr, DMG_BULLET | DMG_NEVERGIB);
			ApplyMultiDamage(pev, pevOwner);

			pev->velocity = Vector(0, 0, 0);

			const char* snd = RANDOM_LONG(0, 1) ? "weapons/xbow_hitbod1.wav" : "weapons/xbow_hitbod2.wav";
			EMIT_SOUND(ENT(pev), CHAN_BODY, snd, 1, ATTN_NORM);
		}
		else
		{
			EMIT_SOUND_DYN(ENT(pev), CHAN_BODY, "weapons/xbow_hit1.wav", RANDOM_FLOAT(0.95, 1.0), ATTN_NORM, 0, 98 + RANDOM_LONG(0, 7));

			SetThink(&CBaseEntity::SUB_Remove);
			pev->nextthink = gpGlobals->time;// this will get changed below if the bolt is allowed to stick in what it hit.

			if (FClassnameIs(pOther->pev, "worldspawn"))
			{
				// if what we hit is static architecture, can stay around for a while.
				Vector vecDir = pev->velocity.Normalize();
				UTIL_SetOrigin(pev, pev->origin - vecDir * 12);
				pev->angles = UTIL_VecToAngles(vecDir);
				pev->solid = SOLID_NOT;
				pev->movetype = MOVETYPE_FLY;
				pev->velocity = Vector(0, 0, 0);
				pev->avelocity.z = 0;
				pev->angles.z = RANDOM_LONG(0, 360);
				pev->nextthink = gpGlobals->time + 10.0;

				pev->starttime = 0;
				pev->impacttime = 0;
			}

			if (UTIL_PointContents(pev->origin) != CONTENTS_WATER) {
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

		return;
	}
};

LINK_ENTITY_TO_CLASS(weapon_cmlwbr, CCmlwbr)
LINK_ENTITY_TO_CLASS(cmlwbr_bolt, CCmlwbrBolt)