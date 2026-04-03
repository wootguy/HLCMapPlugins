#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

// TODO: The hands have fucked up normals and appear flatshaded.
// Same with the original model and vanilla renderer. Why?

enum ElitesAnimation {
	ELITES_IDLE = 0,
	ELITES_IDLE_LEFTEMPTY,
	ELITES_SHOOTLEFT1,
	ELITES_SHOOTLEFT2,
	ELITES_SHOOTLEFT3,
	ELITES_SHOOTLEFT4,
	ELITES_SHOOTLEFT5,
	ELITES_SHOOTLEFTLAST,
	ELITES_SHOOTRIGHT1,
	ELITES_SHOOTRIGHT2,
	ELITES_SHOOTRIGHT3,
	ELITES_SHOOTRIGHT4,
	ELITES_SHOOTRIGHT5,
	ELITES_SHOOTRIGHTLAST,
	ELITES_RELOAD,
	ELITES_DRAW
};

const int ELITES_DEFAULT_GIVE = 30;
const int ELITES_MAX_CARRY = 120;
const int ELITES_MAX_CLIP = 30;
const int ELITES_WEIGHT = 5;

ItemInfo g_glocks_info = {
	4,								// iSlot
	-1,								// iPosition (-1 = automatic)
	"9mm",							// pszAmmo1
	ELITES_MAX_CARRY,				// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"kuilu/weapon_dualglock",		// pszName (path to HUD config)
	ELITES_MAX_CLIP,				// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	ELITES_WEIGHT,					// iWeight
	0,								// iFlagsEx
	6								// accuracy degrees
};

class CDualGlock : public CWeaponCustom {
	int GetItemInfo(ItemInfo* info) {
		*info = g_glocks_info;
		return true;
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_9mmhandgun"; }

	const char* DisplayName() { return "Colt"; }

	void Precache() {
		m_defaultModelV = "models/rngstuff/kuilu/weapons/v_glock.mdl";
		m_defaultModelP = "models/rngstuff/kuilu/weapons/p_glock.mdl";
		m_defaultModelW = "models/rngstuff/kuilu/weapons/w_glock.mdl";
		CBasePlayerWeapon::Precache();

		int shootSnd = PRECACHE_SOUND("rng/kuilu/weapons/colt_fire.wav");
		int reloadSnd1 = PRECACHE_SOUND("rng/kuilu/weapons/glock_reloadstart.wav");
		int reloadSnd2 = PRECACHE_SOUND("rng/kuilu/weapons/glock_clipout.wav");
		int reloadSnd3 = PRECACHE_SOUND("rng/kuilu/weapons/glock_clipin_right.wav");
		int reloadSnd4 = PRECACHE_SOUND("rng/kuilu/weapons/glock_clipin_left.wav");
		int reloadSnd5 = PRECACHE_SOUND("rng/kuilu/weapons/glock_slide.wav");
		int deploySnd = PRECACHE_SOUND("rng/kuilu/weapons/glock_draw.wav");

		int shell = PRECACHE_MODEL("models/shell.mdl");

		PRECACHE_HUD_FILES("sprites/kuilu/weapon_dualglock.txt");

		animExt = "uzis";
		wrongClientWeapon = "weapon_9mmhandgun";

		params.flags = FL_WC_WEP_HAS_PRIMARY;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = ELITES_DRAW;
		params.deployTime = 1100;
		params.deployAnimTime = 1110;
		params.maxClip = ELITES_MAX_CLIP;
		params.reloadStage[0] = { ELITES_RELOAD, 4600 };
		params.idles[0] = { ELITES_IDLE, 99, 1500 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 140;
		primary.accuracyX = 6 * 100;
		primary.accuracyY = 6 * 100;

		float spread = VECTOR_CONE_6DEGREES.x;
		int damage = 36;

		AddEvent(WepEvt().PrimaryEven().WepAnim(ELITES_SHOOTLEFT1));
		AddEvent(WepEvt().PrimaryEven().EjectShell(shell, 26, -8, 9));
		AddEvent(WepEvt().PrimaryOdd().WepAnim(ELITES_SHOOTRIGHT1));
		AddEvent(WepEvt().PrimaryOdd().EjectShell(shell, 26, -8, 9));
		AddEvent(WepEvt().PrimaryClip(1).WepAnim(ELITES_SHOOTLEFTLAST));
		AddEvent(WepEvt().PrimaryClip(0).WepAnim(ELITES_SHOOTRIGHTLAST));

		// prevent the default idle animation playing so the slide doesn't reset.
		AddEvent(WepEvt().PrimaryClip(1).Cooldown(65535, FL_WC_COOLDOWN_IDLE));

		AddEvent(WepEvt().Primary().PlaySound(shootSnd, CHAN_WEAPON, 0.9f, ATTN_NORM, 100, 100, DISTANT_9MM, WC_AIVOL_NORMAL));
		AddEvent(WepEvt().Primary().Bullets(1, 0, damage, spread, spread, 2, WC_FLASH_NORMAL, 0));

		AddEvent(WepEvt().Reload().Delay(0).IdleSound(reloadSnd1));
		AddEvent(WepEvt().Reload().Delay(570).IdleSound(reloadSnd2));
		AddEvent(WepEvt().Reload().Delay(1710).IdleSound(reloadSnd3));
		AddEvent(WepEvt().Reload().Delay(2810).IdleSound(reloadSnd4));
		AddEvent(WepEvt().Reload().Delay(3860).IdleSound(reloadSnd5));

		AddEvent(WepEvt().BulletFired().PunchRandom(2.0f, 0));

		AddEvent(WepEvt().Deploy().IdleSound(deploySnd));

		UTIL_PrecacheOther("ammo_9mmclip");

		PrecacheEvents();
	}

	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
		ammoEntName = "ammo_9mmclip";
		dropAmount = 15;
	}

	void WeaponIdleCustom() {
		if (m_iClip == 1) {
			SendWeaponAnim(ELITES_IDLE_LEFTEMPTY, -1);
			m_flTimeWeaponIdle = 9999; // prevent default idle anim
		}
	}
};


LINK_ENTITY_TO_CLASS(weapon_dualglock, CDualGlock);