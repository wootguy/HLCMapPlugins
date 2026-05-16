#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "CCrowbar.h"
#include "te_effects.h"
#include "skill.h"

const float PIPE_MOD_DAMAGE = 25.0;
const float PIPE_MOD_FATIGUE_TICK = 0.50; // Time between fatigue checks

enum pipe_e
{
	PIPE_IDLE = 0,
	PIPE_DRAW,
	PIPE_HOLSTER,
	PIPE_ATTACK1HIT,
	PIPE_ATTACK1MISS,
	PIPE_ATTACK2MISS,
	PIPE_ATTACK2HIT,
	PIPE_ATTACK3MISS,
	PIPE_ATTACK3HIT,
	PIPE_IDLE2,
	PIPE_IDLE3
};

class CHeaterPipe : public CWeaponCustom {
public:
	TraceResult m_trHit;
	float m_flDmgMulti;
	float m_flDelayMulti;
	int m_iSwingCount;
	bool m_bIsBreathing;

	void Precache() {
		PRECACHE_SOUND("poke646/weapons/heaterpipe/heaterpipe_hit1.wav");
		PRECACHE_SOUND("poke646/weapons/heaterpipe/heaterpipe_hit2.wav");
		PRECACHE_SOUND("poke646/weapons/heaterpipe/heaterpipe_miss.wav");
		PRECACHE_SOUND("poke646/weapons/heaterpipe/heaterpipe_breathe.wav");
		PRECACHE_SOUND("weapons/cbar_hitbod1.wav");
		PRECACHE_SOUND("weapons/cbar_hitbod2.wav");
		PRECACHE_SOUND("weapons/cbar_hitbod3.wav");

		CWeaponCustom::Precache();
	}

	void Smack()
	{
		DecalGunshot(&m_trHit, BULLET_PLAYER_CROWBAR);
	}

	int Deploy() override {
		m_flDmgMulti = 1.0;
		m_flDelayMulti = 0.55;
		m_iSwingCount = 0;

		return CWeaponCustom::Deploy();
	}

	void StopBreathing() {
		CBasePlayer* m_pPlayer = GetPlayer();
		if (!m_pPlayer)
			return;

		if (m_bIsBreathing)
		{
			STOP_SOUND(m_pPlayer->edict(), CHAN_VOICE, "poke646/weapons/heaterpipe/heaterpipe_breathe.wav");
			m_bIsBreathing = false;
		}
	}

	void WeaponIdleCustom() override {
		CBasePlayer* m_pPlayer = GetPlayer();
		if (!m_pPlayer)
			return;

		if (m_iSwingCount != 0)
		{
			--m_iSwingCount;
		}

		if (m_iSwingCount <= 5)
			StopBreathing();

		m_flTimeWeaponIdle = PIPE_MOD_FATIGUE_TICK;
	}

	void Holster(int skiplocal) override {
		CBasePlayer* m_pPlayer = GetPlayer();
		if (!m_pPlayer)
			return;

		CWeaponCustom::Holster(skiplocal);

		StopBreathing();
	}

	void PrimaryAttackCustom() override {
		CBasePlayer* m_pPlayer = GetPlayer();
		if (!m_pPlayer)
			return;

		if (m_iSwingCount == 0) // No fatigue, full dmg/speed
		{
			m_flDmgMulti = 1.0;
			m_flDelayMulti = 0.45;
		}
		else if (m_iSwingCount == 1)
		{
			m_flDmgMulti = 0.95;
			m_flDelayMulti = 0.48;
		}
		else if (m_iSwingCount == 2)
		{
			m_flDmgMulti = 0.80;
			m_flDelayMulti = 0.51;
		}
		else if (m_iSwingCount == 3)
		{
			m_flDmgMulti = 0.70;
			m_flDelayMulti = 0.54;
		}
		else if (m_iSwingCount == 4)
		{
			m_flDmgMulti = 0.60;
			m_flDelayMulti = 0.57;
		}
		else if (m_iSwingCount == 5)
		{
			m_flDmgMulti = 0.50;
			m_flDelayMulti = 0.65;
		}
		else if (m_iSwingCount == 6)
		{
			m_flDmgMulti = 0.50;
			m_flDelayMulti = 0.70;
		}
		else if (m_iSwingCount == 7)
		{
			m_flDmgMulti = 0.50;
			m_flDelayMulti = 0.80;

			if (!m_bIsBreathing) // Check if player is breathing yet
			{
				m_bIsBreathing = true;
				EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_VOICE, "poke646/weapons/heaterpipe/heaterpipe_breathe.wav", 1.0f, ATTN_IDLE, 0, 100);
			}
		}
		else if (m_iSwingCount == 8)
		{
			m_flDmgMulti = 0.50;
			m_flDelayMulti = 0.90;
		}
		else if (m_iSwingCount == 9)
		{
			m_flDmgMulti = 0.55;
			m_flDelayMulti = 1.0;
		}
		else // Maximum exertion
		{
			m_flDmgMulti = 0.50;
			m_flDelayMulti = 1.10;
		}

		m_iSwingCount += 1; // Increment the fatigue counter
		if (m_iSwingCount > 10)
			m_iSwingCount = 10; // We can't go above max fatigue

		int fDidHit = FALSE;

		TraceResult tr;

		UTIL_MakeVectors(m_pPlayer->pev->v_angle);
		Vector vecSrc = m_pPlayer->GetGunPosition();
		Vector vecEnd = vecSrc + gpGlobals->v_forward * 32;

		SolidifyNearbyCorpses(false);

		lagcomp_begin(m_pPlayer);

		UTIL_TraceLine(vecSrc, vecEnd, dont_ignore_monsters, ENT(m_pPlayer->pev), &tr);

		if (tr.flFraction >= 1.0)
		{
			UTIL_TraceHull(vecSrc, vecEnd, dont_ignore_monsters, head_hull, ENT(m_pPlayer->pev), &tr);
			if (tr.flFraction < 1.0)
			{
				// Calculate the point of intersection of the line (or hull) and the object we hit
				// This is and approximation of the "best" intersection
				CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
				if (!pHit || pHit->IsBSPModel())
					FindHullIntersection(vecSrc, tr, VEC_DUCK_HULL_MIN, VEC_DUCK_HULL_MAX, m_pPlayer->edict());
				vecEnd = tr.vecEndPos;	// This is the point on the actual surface (the hull could have hit space)
			}
		}

		m_pPlayer->WaterSplashTrace(vecSrc, 32, head_hull, 0.4f);

		lagcomp_end();

		SolidifyNearbyCorpses(true);

		if (tr.flFraction >= 1.0) {
			Cooldown(0, m_flDelayMulti * 1000);
		}
		else {
			switch (events.animCount % 3)
			{
			case 0:
				SendWeaponAnim(PIPE_ATTACK1HIT, -1); break;
			case 1:
				SendWeaponAnim(PIPE_ATTACK2HIT, -1); break;
			case 2:
				SendWeaponAnim(PIPE_ATTACK3HIT, -1); break;
			}

			// hit
			fDidHit = TRUE;
			CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

			ClearMultiDamage();
			float dmg = GetDamage(PIPE_MOD_DAMAGE * m_flDmgMulti);
			pEntity->TraceAttack(m_pPlayer->pev, dmg, gpGlobals->v_forward, &tr, DMG_CLUB);
			ApplyMultiDamage(m_pPlayer->pev, m_pPlayer->pev);

			// play thwack, smack, or dong sound
			float flVol = 1.0;
			int fHitWorld = TRUE;

			if (pEntity)
			{
				bool suitableClass = pEntity->Classify() != CLASS_NONE || pEntity->IsPlayerCorpse();
				if (suitableClass && !pEntity->IsMachine() && !pEntity->IsBSPModel())
				{
					// play thwack or smack sound
					switch (RANDOM_LONG(0, 2))
					{
					case 0:
						EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod1.wav", 1, ATTN_NORM); break;
					case 1:
						EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod2.wav", 1, ATTN_NORM); break;
					case 2:
						EMIT_SOUND(ENT(m_pPlayer->pev), CHAN_ITEM, "weapons/cbar_hitbod3.wav", 1, ATTN_NORM); break;
					}
					m_pPlayer->m_iWeaponVolume = CROWBAR_BODYHIT_VOLUME;
					flVol = 0.1;
					fHitWorld = FALSE;
				}
			}

			// play texture hit sound
			// UNDONE: Calculate the correct point of intersection when we hit with the hull instead of the line

			if (fHitWorld)
			{
				float fvolbar = TEXTURETYPE_PlaySound(&tr, vecSrc, vecSrc + (vecEnd - vecSrc) * 2, BULLET_PLAYER_CROWBAR);

				// also play crowbar strike
				switch (RANDOM_LONG(0, 1))
				{
				case 0:
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "poke646/weapons/heaterpipe/heaterpipe_hit1.wav", 1, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
					break;
				case 1:
					EMIT_SOUND_DYN(ENT(m_pPlayer->pev), CHAN_ITEM, "poke646/weapons/heaterpipe/heaterpipe_hit2.wav", 1, ATTN_NORM, 0, 98 + RANDOM_LONG(0, 3));
					break;
				}

				// delay the decal a bit
				m_trHit = tr;
			}

			m_pPlayer->m_iWeaponVolume = flVol * CROWBAR_WALLHIT_VOLUME;
			Cooldown(0, m_flDelayMulti*1000);

			SetThink(&CHeaterPipe::Smack);
			pev->nextthink = gpGlobals->time + 0.2;
		}
	}
};

LINK_ENTITY_TO_CLASS(weapon_heaterpipe, CHeaterPipe)
LINK_ENTITY_TO_CLASS(weapon_leadpipe, CHeaterPipe)