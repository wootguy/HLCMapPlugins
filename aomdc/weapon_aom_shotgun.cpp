#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "skill.h"
#include "te_effects.h"

const int SHOTGUN_MAX_CLIP = 8;
const int AOM_SHOTGUN_WEIGHT = 15;

ItemInfo g_shotgun_info = {
	2,								// iSlot
	4,								// iPosition (-1 = automatic)
	"buckshot",						// pszAmmo1
	(int)gSkillData.sk_ammo_max_buckshot,// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"aomdc/weapon_aom_shotgun",			// pszName (path to HUD config)
	SHOTGUN_MAX_CLIP,				// iMaxClip
	-1,								// iId (-1 = automatic)
	ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY,	// iFlags
	AOM_SHOTGUN_WEIGHT,				// iWeight
	0,								// iFlagsEx
	8								// accuracy degrees
};

enum ShotgunAnim
{
	SHOTGUN_IDLE = 0,
	SHOTGUN_LONG_IDLE,
	SHOTGUN_FIRE,
	SHOTGUN_RELOAD_START,
	SHOTGUN_RELOAD_INSERT,
	SHOTGUN_RELOAD_END,
	SHOTGUN_DEPLOY,
};

class CAomShotgun : public CWeaponCustom
{
	int m_iShell;

	void Spawn() {
		m_iDefaultAmmo = 4;
		m_iId = g_shotgun_info.iId;
		CWeaponCustom::Spawn();
	}

	int GetItemInfo(ItemInfo* p) {
		*p = g_shotgun_info;
		return 1;
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_shotgun"; }

	const char* DisplayName() { return "AOM Shotgun"; }

	void Precache() {
		m_defaultModelV = "models/aomdc/v_shotgun.mdl";
		m_defaultModelP = "models/aomdc/p_shotgun.mdl";
		m_defaultModelW = "models/aomdc/w_shotgun.mdl";
		CBasePlayerWeapon::Precache();

		m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");// shotgun shell

		int shootSnd = PRECACHE_SOUND("aomdc/weapons/shotgun_fire.wav");
		int reloadSnd1 = PRECACHE_SOUND("aomdc/weapons/shotgun_insert.wav");
		int reloadSnd2 = PRECACHE_SOUND("aomdc/weapons/shotgun_pump.wav");

		PRECACHE_HUD_FILES("sprites/aomdc/weapon_aom_shotgun.txt");

		animExt = "shotgun";
		wrongClientWeapon = "weapon_shotgun";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_SHOTGUN_RELOAD | FL_WC_WEP_USE_ONLY;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = SHOTGUN_DEPLOY;
		params.deployAnimTime = 520;
		params.maxClip = SHOTGUN_MAX_CLIP;
		params.reloadStage[0] = { SHOTGUN_RELOAD_START, 470 };
		params.reloadStage[1] = { SHOTGUN_RELOAD_INSERT, 530 };
		params.reloadStage[2] = { SHOTGUN_RELOAD_END, 800 };
		params.idles[0] = { SHOTGUN_IDLE, 80, 1000 };
		params.idles[1] = { SHOTGUN_LONG_IDLE, 20, 2700 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 1310;
		primary.accuracyX = 8 * 100;
		primary.accuracyY = 8 * 100;

		float spread = VECTOR_CONE_8DEGREES.x;
		int bulletf = 0;

		AddEvent(WepEvt().Primary().Bullets(8, 0, gSkillData.sk_plr_buckshot, spread, spread, 0, WC_FLASH_NORMAL, bulletf));
		AddEvent(WepEvt().Primary().WepAnim(SHOTGUN_FIRE));
		AddEvent(WepEvt().Primary().Delay(650).EjectShell(m_iShell, 30, -35, 6));
		AddEvent(WepEvt().Primary().Delay(500).IdleSound(reloadSnd2));

		AddEvent(WepEvt().BulletFired().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 100, 100, DISTANT_556, WC_AIVOL_NORMAL));

		AddEvent(WepEvt().Reload().Delay(16)
			.PlaySound(reloadSnd1, CHAN_WEAPON, 1.0f, ATTN_IDLE, 100, 100, DISTANT_NONE, WC_AIVOL_SILENT));

		UTIL_PrecacheOther("ammo_buckshot");

		PrecacheEvents();
	}

	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
		ammoEntName = "ammo_buckshot";
		dropAmount = 4;
	}
};

LINK_ENTITY_TO_CLASS(weapon_aom_shotgun, CAomShotgun);
