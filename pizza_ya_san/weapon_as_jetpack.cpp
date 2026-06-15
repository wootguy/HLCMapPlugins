#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

const int JETPACK_MAX_CLIP = 10;

const float LOW_GRAVITY = 0.3;
const float NORMAL_GRAVITY = 1.0;

const int FUEL_CYCLE = 60;

class CJetpack : public CWeaponCustom {
	float m_flNextAnimTime;
	int m_iSecondaryAmmo;
	int m_iFuel; // 燃料
	// 爆発＆スモーク用
	int m_iBurnSound;
	int m_gBurnSprite;
	int m_gSmokeSprite;

	float m_gravityLowTime; // 重力軽減時間用

	void Spawn() {
		CWeaponCustom::Spawn();
		m_iSecondaryAmmoType = 0;
		m_iClip = 5;
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_9mmhandgun"; }

	int getBoostForce() {
		return 30 + m_iClip * 3;
	}

	void Precache() {
		//These are played by the model, needs changing there
		PRECACHE_SOUND("hl/items/clipinsert1.wav");
		PRECACHE_SOUND("hl/items/cliprelease1.wav");
		PRECACHE_SOUND("hl/items/guncock1.wav");

		PRECACHE_SOUND("weapons/357_cock1.wav");

		// 噴射サウンド
		PRECACHE_SOUND("ambience/steamburst1.wav");
		// 噴射
		m_gBurnSprite = PRECACHE_MODEL("sprites/xflare2.spr");
		m_gSmokeSprite = PRECACHE_MODEL("sprites/boom3.spr");

		CWeaponCustom::Precache();
	}

	int AddToPlayer(CBasePlayer* pPlayer) {
		// resend event data in case the player altered boost levels and picked up a new gun
		bool refreshPredData = UTIL_HasCustomWeaponPredictionData(pPlayer->edict(), this);

		if (CWeaponCustom::AddToPlayer(pPlayer)) {
			if (refreshPredData)
				UTIL_SendCustomWeaponPredictionData(pPlayer->edict(), this, WC_PRED_SEND_EVT);

			return 1;
		}

		return 0;
	}

	BOOL Deploy() {
		m_iFuel = 0;
		m_iBurnSound = 0;
		return CWeaponCustom::Deploy();
	}

	void Holster(int skiplocal) {
		CBasePlayer* m_pPlayer = (CBasePlayer*)m_hPlayer.GetEntity();
		if (!m_pPlayer)
			return;

		m_pPlayer->pev->gravity = NORMAL_GRAVITY;
		return CWeaponCustom::Holster(1);
	}

	// セカンダリアタック
	void SecondaryAttack() {
		CBasePlayer* m_pPlayer = (CBasePlayer*)m_hPlayer.GetEntity();
		if (!m_pPlayer)
			return;

		/*
		// 水中は飛行不可、また燃料なしは飛ばない
		if ((m_pPlayer->pev->waterlevel == WATERLEVEL_HEAD)
			|| (m_pPlayer->rgAmmo(m_iSecondaryAmmoType) <= 0)) {
			PlayEmptySound();
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.15;
			return;
		}

		m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.01;

		// 加速
		float accX = m_pPlayer->pev->velocity.x;
		accX = (accX < 10) ? 0 : 0.15;
		float accY = m_pPlayer->pev->velocity.y;
		accY = (accY < 10) ? 0 : 0.15;

		// clipをPowerLevelにしている。つまり、これによって出力調整
		m_pPlayer->pev->velocity = m_pPlayer->pev->velocity
			+ 10 * Vector(accX, accY, 1)
			+ (3 * m_iClip) * Vector(0, 0, 1);

		// 燃料減
		m_iFuel++;
		m_iFuel %= FUEL_CYCLE;
		if (m_iFuel == 1) {
			m_pPlayer->rgAmmo(m_iSecondaryAmmoType, m_pPlayer->rgAmmo(m_iSecondaryAmmoType) - 1);
		}
		*/

		CWeaponCustom::SecondaryAttack();

		if ((m_pPlayer->pev->waterlevel == WATERLEVEL_HEAD)
			|| (m_pPlayer->rgAmmo(m_iSecondaryAmmoType) <= 0)) {
			return;
		}

		//m_pPlayer->pev->gravity = LOW_GRAVITY;
		//m_gravityLowTime = gpGlobals->time;

		// 発射音＆スプライト
		m_iBurnSound++;
		m_iBurnSound %= (FUEL_CYCLE / 2);
		if (m_iBurnSound == 1) {
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_ITEM,
				"ambience/steamburst1.wav", 0.8, ATTN_NORM, 0, 95 + RANDOM_LONG(0, 10));

			uint8 scale = 150;

			// 爆発
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL);
			WRITE_BYTE(TE_EXPLOSION);
			WRITE_COORD(m_pPlayer->pev->origin.x);
			WRITE_COORD(m_pPlayer->pev->origin.y);
			WRITE_COORD(m_pPlayer->pev->origin.z);
			WRITE_SHORT(m_gBurnSprite);
			WRITE_BYTE(15); // sacle
			WRITE_BYTE(50); // framerate
			WRITE_BYTE(4);  // flag 1=不透明、2=発光なし、4=音なし、8=パーティクルなし
			MESSAGE_END();

			// 煙
			MESSAGE_BEGIN(MSG_BROADCAST, SVC_TEMPENTITY, NULL);
			WRITE_BYTE(TE_SMOKE);
			WRITE_COORD(m_pPlayer->pev->origin.x);
			WRITE_COORD(m_pPlayer->pev->origin.y);
			WRITE_COORD(m_pPlayer->pev->origin.z);
			WRITE_SHORT(m_gSmokeSprite);
			WRITE_BYTE(scale);    // scale
			WRITE_BYTE(10); // framerate
			MESSAGE_END();

		}
	}

	void Reload() {
		CBasePlayer* m_pPlayer = (CBasePlayer*)m_hPlayer.GetEntity();
		if (!m_pPlayer)
			return;

		// リロードといいつつ、出力レベル調整。clipが出力レベル
		if ((UTIL_WeaponTimeBase() > m_flNextPrimaryAttack)
			&& (UTIL_WeaponTimeBase() > m_flNextSecondaryAttack)
			) {
			// しゃがみ中で-1、立ち状態で+1
			int pitch = 100;
			if ((m_pPlayer->pev->button & IN_DUCK) != 0) {
				m_iClip--;
				m_iClip = (m_iClip < 1) ? JETPACK_MAX_CLIP : m_iClip;
				pitch = 90;
			}
			else {
				m_iClip++;
				m_iClip = (m_iClip > JETPACK_MAX_CLIP) ? 1 : m_iClip;
				pitch = 110;
			}
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON, "weapons/357_cock1.wav", 1.0, ATTN_NORM, 0, pitch);

			m_flNextPrimaryAttack = UTIL_WeaponTimeBase() + 0.5;
			m_flNextSecondaryAttack = UTIL_WeaponTimeBase() + 0.5;

			// update prediction data for boost force
			for (int i = 0; i < defaultParams.numEvents; i++) {
				WepEvt& evt = defaultParams.events[i];
				if (evt.evtType == WC_EVT_KICKBACK) {
					evt.kickback.pushForce = getBoostForce();
				}
			}

			UTIL_SendCustomWeaponPredictionData(m_pPlayer->edict(), this, WC_PRED_SEND_EVT);

			UTIL_ClientPrint(m_pPlayer, print_center, UTIL_VarArgs("Boost Level: %d", m_iClip));
		}
	}

	void WeaponIdle() {
		CBasePlayer* m_pPlayer = (CBasePlayer*)m_hPlayer.GetEntity();
		if (!m_pPlayer)
			return;

		/*
		if (gpGlobals->time > m_gravityLowTime + 1.0) {
			m_pPlayer->pev->gravity = NORMAL_GRAVITY;
		}
		*/

		CWeaponCustom::WeaponIdle();
	}
};


LINK_ENTITY_TO_CLASS(weapon_as_jetpack, CJetpack);