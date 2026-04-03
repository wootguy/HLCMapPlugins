#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

enum GlockAnimation {
	FIVE7_IDLE = 0,
	FIVE7_SHOOT1,
	FIVE7_SHOOT2,
	FIVE7_SHOOTLAST,
	FIVE7_RELOAD,
	FIVE7_DRAW
};

const int FIVE7_DEFAULT_GIVE = 34;
const int FIVE7_MAX_CARRY = 100;
const int FIVE7_MAX_CLIP = 17;
const int FIVE7_WEIGHT = 5;

ItemInfo g_fiveseven_info = {
	1,								// iSlot
	-1,								// iPosition (-1 = automatic)
	"9mm",							// pszAmmo1
	FIVE7_MAX_CARRY,				// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"kuilu/weapon_fiveseven",		// pszName (path to HUD config)
	FIVE7_MAX_CLIP,					// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	FIVE7_WEIGHT,					// iWeight
	0,								// iFlagsEx
	1								// accuracy degrees
};

class CFiveSeven : public CWeaponCustom {
	int GetItemInfo(ItemInfo* info) {
		*info = g_fiveseven_info;
		return true;
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_9mmhandgun"; }

	const char* DisplayName() { return "Five Seven"; }

	void Precache() {
		m_defaultModelV = "models/rngstuff/kuilu/weapons/v_fiveseven.mdl";
		m_defaultModelP = "models/rngstuff/kuilu/weapons/p_fiveseven.mdl";
		m_defaultModelW = "models/rngstuff/kuilu/weapons/w_fiveseven.mdl";
		CBasePlayerWeapon::Precache();

		int shootSnd = PRECACHE_SOUND("rng/kuilu/weapons/fiveseven_fire.wav");
		int reloadSnd1 = PRECACHE_SOUND("rng/kuilu/weapons/fiveseven_clipout.wav");
		int reloadSnd2 = PRECACHE_SOUND("rng/kuilu/weapons/fiveseven_clipin.wav");
		int reloadSnd3 = PRECACHE_SOUND("rng/kuilu/weapons/fiveseven_slide.wav");
		int deploySnd = PRECACHE_SOUND("rng/kuilu/weapons/fiveseven_draw.wav");
		int deploySnd2 = PRECACHE_SOUND("rng/kuilu/weapons/fiveseven_slidepull.wav");

		int shell = PRECACHE_MODEL("models/shell.mdl");

		PRECACHE_HUD_FILES("sprites/kuilu/weapon_fiveseven.txt");

		animExt = "onehanded";
		wrongClientWeapon = "weapon_9mmhandgun";

		params.flags = FL_WC_WEP_HAS_PRIMARY;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = FIVE7_DRAW;
		params.deployTime = 1000;
		params.deployAnimTime = 1070;
		params.maxClip = FIVE7_MAX_CLIP;
		params.reloadStage[0] = { FIVE7_RELOAD, 3240 };
		params.idles[0] = { FIVE7_IDLE, 99, 500 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 160;
		primary.accuracyX = 1 * 100;
		primary.accuracyY = 1 * 100;

		float spread = VECTOR_CONE_1DEGREES.x;
		int damage = 24;

		AddEvent(WepEvt().Primary().WepAnim(FIVE7_SHOOT1));
		AddEvent(WepEvt().Primary().PlaySound(shootSnd, CHAN_WEAPON, 0.9f, ATTN_NORM, 100, 100, DISTANT_9MM, WC_AIVOL_NORMAL));
		AddEvent(WepEvt().Primary().Bullets(1, 0, damage, spread, spread, 2, WC_FLASH_NORMAL, 0));

		AddEvent(WepEvt().Reload().Delay(500).IdleSound(reloadSnd1));
		AddEvent(WepEvt().Reload().Delay(1450).IdleSound(reloadSnd2));
		AddEvent(WepEvt().Reload().Delay(2500).IdleSound(reloadSnd3));

		AddEvent(WepEvt().BulletFired().PunchRandom(2.0f, 0));
		AddEvent(WepEvt().BulletFired().EjectShell(shell, 26, -8, 9));

		AddEvent(WepEvt().Deploy().IdleSound(deploySnd));
		AddEvent(WepEvt().Deploy().Delay(430).IdleSound(deploySnd2));

		UTIL_PrecacheOther("ammo_9mmclip");

		PrecacheEvents();
	}

	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
		ammoEntName = "ammo_9mmclip";
		dropAmount = FIVE7_MAX_CLIP;
	}
};


LINK_ENTITY_TO_CLASS(weapon_fiveseven, CFiveSeven);