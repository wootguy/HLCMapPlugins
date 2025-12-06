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

ItemInfo g_heaterpipe_info = {
	0,								// iSlot
	-1,								// iPosition (-1 = automatic)
	NULL,							// pszAmmo1
	-1,								// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"poke646/weapon_heaterpipe",	// pszName (path to HUD config)
	-1,								// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	0,								// iWeight
	0,								// iFlagsEx
	0,								// accuracy degrees
};

class CHeaterPipe : public CWeaponCustom {
public:
	TraceResult m_trHit;
	float m_flDmgMulti;
	float m_flDelayMulti;
	int m_iSwingCount;
	bool m_bIsBreathing;

	void Spawn()
	{		m_iId = g_heaterpipe_info.iId;
		CWeaponCustom::Spawn();
	}

	virtual void PrecacheWeaponModels() {
		m_defaultModelV = "models/poke646/weapons/heaterpipe/v_heaterpipe.mdl";
		m_defaultModelP = "models/poke646/weapons/heaterpipe/p_heaterpipe.mdl";
		m_defaultModelW = "models/poke646/weapons/heaterpipe/w_heaterpipe.mdl";
		CBasePlayerWeapon::Precache();
	}

	void Precache() {
		PrecacheWeaponModels();

		int hitSnd1 = PRECACHE_SOUND("poke646/weapons/heaterpipe/heaterpipe_hit1.wav");
		int hitSnd2 = PRECACHE_SOUND("poke646/weapons/heaterpipe/heaterpipe_hit2.wav");
		int missSnd = PRECACHE_SOUND("poke646/weapons/heaterpipe/heaterpipe_miss.wav");
		int breatheSnd = PRECACHE_SOUND("poke646/weapons/heaterpipe/heaterpipe_breathe.wav");
		int hitBodySnd1 = PRECACHE_SOUND("weapons/cbar_hitbod1.wav");
		int hitBodySnd2 = PRECACHE_SOUND("weapons/cbar_hitbod2.wav");
		int hitBodySnd3 = PRECACHE_SOUND("weapons/cbar_hitbod3.wav");

		PRECACHE_HUD_FILES("sprites/poke646/weapon_heaterpipe.txt");

		animExt = "crowbar";
		//wrongClientWeapon = "weapon_crowbar";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_NO_PREDICTION;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = PIPE_DRAW;
		params.deployAnimTime = 630;
		params.idles[0] = { PIPE_IDLE, 100, 1000 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 0;
		primary.cooldown = 500;
		primary.flags = FL_WC_SHOOT_UNDERWATER;
		int bulletf = FL_WC_BULLETS_NO_DECAL;

		AddEvent(WepEvt().Primary().WepAnim(PIPE_ATTACK1MISS, 0, FL_WC_ANIM_ORDERED)
			.AddAnim(PIPE_ATTACK2MISS).AddAnim(PIPE_ATTACK3MISS));
		AddEvent(WepEvt().Primary().PlaySound(missSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 94, 109, DISTANT_NONE, WC_AIVOL_QUIET));

		PrecacheEvents();
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_heaterpipe_info;
		return true;
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
			switch (animCount % 3)
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


class CLeadPipe : public CHeaterPipe {

	void Spawn()
	{
		pev->classname = ALLOC_STRING("weapon_heaterpipe");
		m_iId = g_heaterpipe_info.iId;
		g_heaterpipe_info.pszName = "vendetta/weapon_heaterpipe.txt";
		m_customSpriteDir = ALLOC_STRING("vendetta");
		CWeaponCustom::Spawn();
	}

	void Precache() {
		CHeaterPipe::Precache();
		PRECACHE_HUD_FILES("sprites/vendetta/weapon_heaterpipe.txt");
	}

	void PrecacheWeaponModels() override {
		m_defaultModelV = "models/vendetta/weapons/leadpipe/v_leadpipe.mdl";
		m_defaultModelP = "models/vendetta/weapons/leadpipe/p_leadpipe.mdl";
		m_defaultModelW = "models/vendetta/weapons/leadpipe/w_leadpipe.mdl";
		CBasePlayerWeapon::Precache();
	}
};

LINK_ENTITY_TO_CLASS(weapon_heaterpipe, CHeaterPipe)
LINK_ENTITY_TO_CLASS(weapon_leadpipe, CLeadPipe)