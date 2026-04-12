#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

enum SawedOffAnimation
{
	TOZ34_IDLE = 0,
	TOZ34_DRAW,
	TOZ34_RELOAD_SINGLE,
	TOZ34_RELOAD,
	TOZ34_SHOOT1
};

const int SAWEDOFF_MAX_CARRY = 125;
const int SAWEDOFF_DEFAULT_GIVE = 4;
const int SAWEDOFF_MAX_CLIP = 2;
const int SAWEDOFF_WEIGHT = 35;
const int SAWEDOFF_SINGLE_PELLETCOUNTER = 24;
const int SAWEDOFF_DOUBLE_PELLETCOUNT = SAWEDOFF_SINGLE_PELLETCOUNTER * 2;
const Vector VECTOR_CONE_DM_SAWEDOFFS(0.10716, 0.12362, 0.00); // 12.3 degrees by 14.2 degrees
const Vector VECTOR_CONE_DM_SAWEDOFFD(0.12716, 0.16365, 0.00); // 14.6 degrees by 18.8 degrees

ItemInfo g_sawedoff_info = {
	2,								// iSlot
	-1,								// iPosition (-1 = automatic)
	"buckshot",						// pszAmmo1
	SAWEDOFF_MAX_CARRY,				// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"kuilu/weapon_sawedoff",		// pszName (path to HUD config)
	SAWEDOFF_MAX_CLIP,				// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	SAWEDOFF_WEIGHT,				// iWeight
	0,								// iFlagsEx
	1								// accuracy degrees
};

class CSawedOff : public CWeaponCustom {
	int GetItemInfo(ItemInfo* info) {
		*info = g_sawedoff_info;
		return true;
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_shotgun"; }

	const char* DisplayName() { return "Sawed-off Shotgun"; }

	void Precache() {
		m_defaultModelV = "models/rngstuff/kuilu/weapons/v_sawedoff.mdl";
		m_defaultModelP = "models/rngstuff/kuilu/weapons/p_sawedoff.mdl";
		m_defaultModelW = "models/rngstuff/kuilu/weapons/w_sawedoff.mdl";
		CBasePlayerWeapon::Precache();

		int shootSnd = PRECACHE_SOUND("rng/kuilu/weapons/shotgun_fire.wav");
		int shootSnd2 = PRECACHE_SOUND("rng/kuilu/weapons/shotgun_fire2.wav");
		int reloadSnd1 = PRECACHE_SOUND("rng/kuilu/weapons/shotgun_open.wav");
		int reloadSnd2 = PRECACHE_SOUND("rng/kuilu/weapons/shotgun_shell_out.wav");
		int reloadSnd3 = PRECACHE_SOUND("rng/kuilu/weapons/shotgun_shell1.wav");
		int reloadSnd4 = PRECACHE_SOUND("rng/kuilu/weapons/shotgun_close.wav");
		int reloadSnd5 = PRECACHE_SOUND("rng/kuilu/weapons/shotgun_shells_eject.wav");
		int reloadSnd6 = PRECACHE_SOUND("rng/kuilu/weapons/shotgun_shell2.wav");
		int deploySnd = PRECACHE_SOUND("rng/kuilu/weapons/shotgun_draw.wav");

		int shell = PRECACHE_MODEL("models/shotgunshell.mdl");

		PRECACHE_HUD_FILES("sprites/kuilu/weapon_sawedoff.txt");

		animExt = "shotgun";
		wrongClientWeapon = "weapon_shotgun";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_HAS_SECONDARY;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = TOZ34_DRAW;
		params.deployTime = 1150;
		params.deployAnimTime = 1150;
		params.maxClip = SAWEDOFF_MAX_CLIP;
		params.reloadStage[0] = { TOZ34_RELOAD_SINGLE, 3620 };
		params.reloadStage[1] = { TOZ34_RELOAD, 3020 };
		params.idles[0] = { TOZ34_IDLE, 100, 2000 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 123;
		primary.accuracyX = 12.3f * 100;
		primary.accuracyY = 14.6f * 100;
		primary.flags = FL_WC_SHOOT_NO_AUTOFIRE;

		CustomWeaponShootOpts& secondary = params.shootOpts[1];
		secondary.ammoCost = 2;
		secondary.ammoPool = WC_AMMOPOOL_PRIMARY_CLIP;
		secondary.cooldown = 600;
		secondary.accuracyX = 14.6f * 100;
		secondary.accuracyY = 18.8f * 100;
		secondary.flags = FL_WC_SHOOT_NEED_FULL_COST;

		// scaled down because HL has fewer pellets than sven
		float damage = roundf((gSkillData.sk_12mm_bullet * 4.0f) / 6.0f);

		AddEvent(WepEvt().Primary().WepAnim(TOZ34_SHOOT1));
		AddEvent(WepEvt().Primary().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 85, 116, DISTANT_9MM, WC_AIVOL_LOUD, 0));
		AddEvent(WepEvt().Primary().Bullets(SAWEDOFF_SINGLE_PELLETCOUNTER, 0, damage,
			VECTOR_CONE_DM_SAWEDOFFS.x, VECTOR_CONE_DM_SAWEDOFFS.y, 0, WC_FLASH_BRIGHT, 0));
		AddEvent(WepEvt().Primary().PunchRandom(1.0f, 0));
		AddEvent(WepEvt().Primary().PunchAdd(-4.0f, 0));

		AddEvent(WepEvt().Secondary().WepAnim(TOZ34_SHOOT1));
		AddEvent(WepEvt().Secondary().PlaySound(shootSnd2, CHAN_WEAPON, 1.0f, ATTN_NORM, 85, 116, DISTANT_9MM, WC_AIVOL_LOUD, 0));
		AddEvent(WepEvt().Secondary().Bullets(SAWEDOFF_DOUBLE_PELLETCOUNT, 0, damage,
			VECTOR_CONE_DM_SAWEDOFFS.x, VECTOR_CONE_DM_SAWEDOFFS.y, 0, WC_FLASH_BRIGHT, 0));
		AddEvent(WepEvt().Secondary().PunchAdd(-10.0f, 0));

		AddEvent(WepEvt().ReloadNotEmpty().Delay(360).IdleSound(reloadSnd1));
		AddEvent(WepEvt().ReloadNotEmpty().Delay(920).IdleSound(reloadSnd2));
		AddEvent(WepEvt().ReloadNotEmpty().Delay(2010).IdleSound(reloadSnd3));
		AddEvent(WepEvt().ReloadNotEmpty().Delay(2760).IdleSound(reloadSnd4));

		AddEvent(WepEvt().ReloadEmpty().Delay(360).IdleSound(reloadSnd1));
		AddEvent(WepEvt().ReloadEmpty().Delay(420).IdleSound(reloadSnd5));
		AddEvent(WepEvt().ReloadEmpty().Delay(1400).IdleSound(reloadSnd3));
		AddEvent(WepEvt().ReloadEmpty().Delay(1680).IdleSound(reloadSnd6));
		AddEvent(WepEvt().ReloadEmpty().Delay(2280).IdleSound(reloadSnd4));

		AddEvent(WepEvt().Deploy().IdleSound(deploySnd));
		AddEvent(WepEvt().Deploy().Delay(350).IdleSound(reloadSnd4));

		UTIL_PrecacheOther("ammo_buckshot");

		PrecacheEvents();
	}

	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
		ammoEntName = "ammo_buckshot";
		dropAmount = 12;
	}
};


LINK_ENTITY_TO_CLASS(weapon_sawedoff, CSawedOff);