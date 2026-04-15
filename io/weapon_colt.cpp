#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

enum ColtAnimation {
	DEAGLE_IDLE = 0,
	DEAGLE_SHOOT1,
	DEAGLE_SHOOT2,
	DEAGLE_EMPTY,
	DEAGLE_RELOAD,
	DEAGLE_DRAW
};

const int DEAGLE_DEFAULT_GIVE = 16;
const int DEAGLE_MAX_CARRY = 40;
const int DEAGLE_MAX_CLIP = 8;
const int DEAGLE_WEIGHT = 7;

ItemInfo g_colt_info = {
	1,								// iSlot
	-1,								// iPosition (-1 = automatic)
	"357",							// pszAmmo1
	NULL,							// pszAmmo2
	"kuilu/weapon_colt",			// pszName (path to HUD config)
	DEAGLE_MAX_CLIP,				// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	DEAGLE_WEIGHT,					// iWeight
	0,								// iFlagsEx
	1								// accuracy degrees
};

class CColt : public CWeaponCustom {
	int GetItemInfo(ItemInfo* info) {
		*info = g_colt_info;
		return true;
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_9mmhandgun"; }

	const char* DisplayName() { return "Colt"; }

	void Precache() {
		m_defaultModelV = "models/rngstuff/kuilu/weapons/v_colt.mdl";
		m_defaultModelP = "models/rngstuff/kuilu/weapons/p_colt.mdl";
		m_defaultModelW = "models/rngstuff/kuilu/weapons/w_colt.mdl";
		CBasePlayerWeapon::Precache();

		int shootSnd = PRECACHE_SOUND("rng/kuilu/weapons/colt_fire.wav");
		int reloadSnd1 = PRECACHE_SOUND("rng/kuilu/weapons/colt_slideback.wav");
		int reloadSnd2 = PRECACHE_SOUND("rng/kuilu/weapons/colt_clipout.wav");
		int reloadSnd3 = PRECACHE_SOUND("rng/kuilu/weapons/colt_clipin.wav");
		int reloadSnd4 = PRECACHE_SOUND("rng/kuilu/weapons/colt_slideforward.wav");
		int deploySnd = PRECACHE_SOUND("rng/kuilu/weapons/colt_draw.wav");

		int shell = PRECACHE_MODEL("models/shell.mdl");

		PRECACHE_HUD_FILES("sprites/kuilu/weapon_colt.txt");

		animExt = "onehanded";
		wrongClientWeapon = "weapon_9mmhandgun";

		params.flags = FL_WC_WEP_HAS_PRIMARY;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = DEAGLE_DRAW;
		params.deployTime = 1000;
		params.deployAnimTime = 1030;
		params.maxClip = DEAGLE_MAX_CLIP;
		params.reloadStage[0] = { DEAGLE_RELOAD, 2200 };
		params.idles[0] = { DEAGLE_IDLE, 99, 1500 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 200;
		primary.accuracyX = 1 * 100;
		primary.accuracyY = 1 * 100;

		float spread = VECTOR_CONE_1DEGREES.x;
		int damage = 80;

		AddEvent(WepEvt().PrimaryNotEmpty().WepAnim(DEAGLE_SHOOT1).AddAnim(DEAGLE_SHOOT2));
		AddEvent(WepEvt().PrimaryEmpty().WepAnim(DEAGLE_EMPTY));
		AddEvent(WepEvt().Primary().PlaySound(shootSnd, CHAN_WEAPON, 0.9f, ATTN_NORM, 100, 100, DISTANT_9MM, WC_AIVOL_NORMAL, 0));
		AddEvent(WepEvt().Primary().Bullets(1, 0, damage, spread, spread, 2, WC_FLASH_NORMAL, 0));

		AddEvent(WepEvt().Reload().Delay(0).IdleSound(reloadSnd1));
		AddEvent(WepEvt().Reload().Delay(460).IdleSound(reloadSnd2));
		AddEvent(WepEvt().Reload().Delay(1130).IdleSound(reloadSnd3));
		AddEvent(WepEvt().Reload().Delay(1630).IdleSound(reloadSnd4));

		AddEvent(WepEvt().BulletFired().PunchRandom(2.0f, 0));
		AddEvent(WepEvt().BulletFired().EjectShell(shell, TE_BOUNCE_SHELL, 26, -8, 9));

		AddEvent(WepEvt().Deploy().IdleSound(deploySnd));

		UTIL_PrecacheOther("ammo_357");

		PrecacheEvents();
	}

	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
		ammoEntName = "ammo_357";
		dropAmount = DEAGLE_MAX_CLIP;
	}
};


LINK_ENTITY_TO_CLASS(weapon_colt, CColt);