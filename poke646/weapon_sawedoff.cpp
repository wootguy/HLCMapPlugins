#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "skill.h"
#include "te_effects.h"

const float SAWEDOFF_DAMAGE = 30.0;
const float SAWEDOFF_MOD_FIRERATE = 1.30;
const float SAWEDOFF_MOD_RELOAD = 0.32;

const int SAWEDOFF_DEFAULT_AMMO = 12;
const int SAWEDOFF_MAX_CARRY = 48;
const int SAWEDOFF_MAX_CLIP = 12;
const int SAWEDOFF_WEIGHT = 15;

ItemInfo g_sawedoff_info = {
	2,								// iSlot
	-1,								// iPosition (-1 = automatic)
	"buckshot",						// pszAmmo1
	(int)gSkillData.sk_ammo_max_buckshot,// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"poke646/weapon_sawedoff",		// pszName (path to HUD config)
	SAWEDOFF_MAX_CLIP,				// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	SAWEDOFF_WEIGHT,				// iWeight
	0,								// iFlagsEx
	10								// accuracy degrees
};

enum SawedOffAnimation
{
	SAWEDOFF_IDLE = 0,
	SAWEDOFF_FIRE,
	SAWEDOFF_INSERT,
	SAWEDOFF_PUMP,
	SAWEDOFF_START_RELOAD,
	SAWEDOFF_DEPLOY,
	SAWEDOFF_HOLSTER,
	SAWEDOFF_IDLE_DEEP
};

class CSawedOff : public CWeaponCustom
{
	int m_iShell;

	void Spawn()
	{
		m_iDefaultAmmo = SAWEDOFF_DEFAULT_AMMO;
		m_iId = g_sawedoff_info.iId;
		CWeaponCustom::Spawn();
	}

	void Precache()
	{
		m_defaultModelV = "models/vendetta/weapons/sawedoff/v_sawedoff.mdl";
		m_defaultModelP = "models/vendetta/weapons/sawedoff/p_sawedoff.mdl";
		m_defaultModelW = "models/vendetta/weapons/sawedoff/w_sawedoff.mdl";
		CBasePlayerWeapon::Precache();

		m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");// shotgun shell

		PRECACHE_SOUND("items/9mmclip1.wav");

		int shootSnd = PRECACHE_SOUND("vendetta/weapons/sawedoff/sawedoff_fire.wav");
		int reloadSnd1 = PRECACHE_SOUND("vendetta/weapons/sawedoff/sawedoff_reload1.wav");
		int reloadSnd2 = PRECACHE_SOUND("vendetta/weapons/sawedoff/sawedoff_reload2.wav");
		int pumpSnd = PRECACHE_SOUND("vendetta/weapons/sawedoff/sawedoff_pump.wav");

		PRECACHE_HUD_FILES("sprites/poke646/weapon_sawedoff.txt");

		animExt = "shotgun";
		wrongClientWeapon = "weapon_shotgun";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_SHOTGUN_RELOAD;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = SAWEDOFF_DEPLOY;
		params.deployAnimTime = 630;
		params.maxClip = SAWEDOFF_MAX_CLIP;
		params.reloadStage[0] = { SAWEDOFF_START_RELOAD, 550 }; // 750 = anim time
		params.reloadStage[1] = { SAWEDOFF_INSERT, 340 }; // 480 = anim time
		params.reloadStage[2] = { SAWEDOFF_PUMP, 1000 };
		params.idles[0] = { SAWEDOFF_IDLE, 70, 5670 };
		params.idles[1] = { SAWEDOFF_IDLE_DEEP, 30, 4250 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 2;
		primary.cooldown = 1150;
		primary.accuracyX = 10 * 100;
		primary.accuracyY = 10 * 100;
		primary.flags = FL_WC_SHOOT_NEED_FULL_COST;

		float spread = VECTOR_CONE_10DEGREES.x;
		int bulletf = 0;

		AddEvent(WepEvt().Primary().Bullets(12, 0, gSkillData.sk_plr_buckshot / 2, spread, spread, 0, WC_FLASH_NORMAL, bulletf));
		AddEvent(WepEvt().Primary().WepAnim(SAWEDOFF_FIRE));
		AddEvent(WepEvt().Primary().EjectShell(m_iShell, 14, -12, 6));
		AddEvent(WepEvt().Primary().EjectShell(m_iShell, 14, -12, 6));

		AddEvent(WepEvt().BulletFired().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 94, 109, DISTANT_556, WC_AIVOL_NORMAL));
		AddEvent(WepEvt().BulletFired().PunchSet(-4, 0));
		AddEvent(WepEvt().BulletFired().Kickback(200));

		AddEvent(WepEvt().Reload().Delay(16)
			.PlaySound(reloadSnd1, CHAN_WEAPON, 1.0f, ATTN_IDLE, 100, 100, DISTANT_NONE, WC_AIVOL_SILENT)
			.AddSound(reloadSnd2));

		AddEvent(WepEvt().Primary().Delay(600).IdleSound(pumpSnd));
		AddEvent(WepEvt().ReloadFinish().Delay(380).IdleSound(pumpSnd));

		PrecacheEvents();
	}

	int GetItemInfo(ItemInfo* p)
	{
		*p = g_sawedoff_info;
		return 1;
	}
};

LINK_ENTITY_TO_CLASS(weapon_sawedoff, CSawedOff);
