#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

/*
 * Jetpack & Glock18
 * (Refeneced: The original Half-Life version of the mp5)
 */
enum GlockAnimation {
	GLOCK_IDLE1 = 0,
	GLOCK_IDLE2,
	GLOCK_IDLE3,
	GLOCK_SHOOT,
	GLOCK_SHOOT_EMPTY,
	GLOCK_RELOAD,
	GLOCK_RELOAD_NOT_EMPTY,
	GLOCK_DRAW,
	GLOCK_HOLSTER,
	GLOCK_ADD_SILENCER
};

const int JETPACK_DEFAULT_GIVE = 100;
const int JETPACK_MAX_AMMO = 250;
const int JETPACK_MAX_AMMO2 = 100;
const int JETPACK_MAX_CLIP = 10;
const int JETPACK_WEIGHT = 5;

const float LOW_GRAVITY = 0.3;
const float NORMAL_GRAVITY = 1.0;

const int FUEL_CYCLE = 60;

ItemInfo g_jetpack_info = {
	1,								// iSlot
	-1,								// iPosition (-1 = automatic)
	"9mm",							// pszAmmo1
	JETPACK_MAX_AMMO,				// iMaxAmmo1
	"uranium",						// pszAmmo2
	JETPACK_MAX_AMMO2,				// iMaxAmmo2
	"pizza_ya_san/weapon_as_jetpack",// pszName (path to HUD config)
	JETPACK_MAX_CLIP,				// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	JETPACK_WEIGHT,					// iWeight
	0,								// iFlagsEx
	6								// accuracy degrees
};

class CJetpack : public CWeaponCustom {
	float m_flNextAnimTime;
	int m_iShell;
	int m_iSecondaryAmmo;
	int m_iFuel; // 燃料
	// 爆発＆スモーク用
	int m_iBurnSound;
	int m_gBurnSprite;
	int m_gSmokeSprite;

	float m_gravityLowTime; // 重力軽減時間用

	void Spawn() {
		Precache();
		SET_MODEL(edict(), "models/pizza_ya_san/w_glock18jet.mdl");

		m_iDefaultAmmo = JETPACK_DEFAULT_GIVE;
		m_iSecondaryAmmoType = 0;
		m_iId = g_jetpack_info.iId;
		FallInit();
		m_iClip = 5;
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_9mmhandgun"; }

	int getBoostForce() {
		return 30 + m_iClip * 3;
	}

	void Precache() {
		m_defaultModelV = "models/pizza_ya_san/v_glock18jet.mdl";
		m_defaultModelP = "models/pizza_ya_san/p_glock18jet.mdl";
		m_defaultModelW = "models/pizza_ya_san/w_glock18jet.mdl";
		CBasePlayerWeapon::Precache();

		PRECACHE_HUD_FILES("sprites/pizza_ya_san/weapon_as_jetpack.txt");

		m_iShell = PRECACHE_MODEL("models/shell.mdl");

		PRECACHE_MODEL("models/w_9mmARclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");

		//These are played by the model, needs changing there
		PRECACHE_SOUND("hl/items/clipinsert1.wav");
		PRECACHE_SOUND("hl/items/cliprelease1.wav");
		PRECACHE_SOUND("hl/items/guncock1.wav");

		int shootSnd = PRECACHE_SOUND("weapons/hks1.wav");
		PRECACHE_SOUND("hl/weapons/357_cock1.wav");

		// 噴射サウンド
		PRECACHE_SOUND("ambience/steamburst1.wav");
		// 噴射
		m_gBurnSprite = PRECACHE_MODEL("sprites/xflare2.spr");
		m_gSmokeSprite = PRECACHE_MODEL("sprites/boom3.spr");

		animExt = "onehanded";
		//wrongClientWeapon = "weapon_shotgun";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_HAS_SECONDARY | FL_WC_WEP_UNLINK_COOLDOWNS;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = GLOCK_DRAW;
		params.deployAnimTime = 840;
		params.maxClip = JETPACK_MAX_CLIP;
		params.idles[0] = { GLOCK_IDLE1, 50, 3100 };
		params.idles[1] = { GLOCK_IDLE2, 50, 1440 };
		//params.idles[2] = { GLOCK_IDLE3, 80, 4670 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.ammoPool = WC_AMMOPOOL_PRIMARY_RESERVE;
		primary.cooldown = 50;
		primary.accuracyX = 6 * 100;
		primary.accuracyY = 6 * 100;

		CustomWeaponShootOpts& secondary = params.shootOpts[1];
		secondary.ammoCost = 1;
		secondary.ammoFreq = 50;
		secondary.cooldown = 20;
		secondary.accuracyX = 6 * 100;
		secondary.accuracyY = 6 * 100;

		float spread = VECTOR_CONE_6DEGREES.x;
		int bulletf = 0;

		AddEvent(WepEvt().Primary().Bullets(1, 0, gSkillData.sk_9mm_bullet, spread, spread, 0, WC_FLASH_NORMAL, bulletf));
		AddEvent(WepEvt().Primary().WepAnim(GLOCK_SHOOT));
		AddEvent(WepEvt().Primary().EjectShell(m_iShell, 14, -12, 6));

		// TODO: Predict gravity changes so this can match the original behavior exactly
		AddEvent(WepEvt().Secondary().Kickback(getBoostForce(), 0, 0, 0, 100));
		//AddEvent(WepEvt().Secondary().SetGravity(LOW_GRAVITY));
		//AddEvent(WepEvt().SecondaryStop().Delay(1000).SetGravity(0));

		AddEvent(WepEvt().BulletFired().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 94, 109, DISTANT_9MM, WC_AIVOL_NORMAL));
		AddEvent(WepEvt().BulletFired().PunchRandom(2, 0));

		PrecacheEvents();
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_jetpack_info;
		return true;
	}

	int AddToPlayer(CBasePlayer* pPlayer) {
		// resend event data in case the player altered boost levels and picked up a new gun
		bool refreshPredData = HasPredictionData(pPlayer->edict());

		if (CWeaponCustom::AddToPlayer(pPlayer)) {
			if (refreshPredData)
				SendPredictionData(pPlayer->edict(), WC_PRED_SEND_EVT);

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
			EMIT_SOUND_DYN(m_pPlayer->edict(), CHAN_WEAPON,
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
			for (int i = 0; i < params.numEvents; i++) {
				WepEvt& evt = params.events[i];
				if (evt.evtType == WC_EVT_KICKBACK) {
					evt.kickback.pushForce = getBoostForce();
				}
			}
			SendPredictionData(m_pPlayer->edict(), WC_PRED_SEND_EVT);

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

	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
		ammoEntName = "ammo_uziclip";
		dropAmount = 32;
	}
};


LINK_ENTITY_TO_CLASS(weapon_as_jetpack, CJetpack);