#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "skill.h"
#include "te_effects.h"

/*
 * HLベース改造ショットガン
 *
 * fgdファイルに下記を追加して、エンティティとしてマップに仕込むこと。
 * @PointClass base(Weapon, Targetx, ExclusiveHold) studio("models/pizza_ya_san/w_shotgun_shorty.mdl") = weapon_as_shotgun : "custom shotgun" []
 */
const Vector VECTOR_CONE_DM_SHOTGUN(0.13074, 0.13074, 0.00);		// 15 degrees

#define AS_SHOTGUN_DEFAULT_AMMO 12
#define AS_SHOTGUN_MAX_CARRY 125
#define AS_SHOTGUN_MAX_CLIP 4
#define AS_SHOTGUN_WEIGHT 15

#define AS_SHOTGUN_PELLETCOUNT 9

ItemInfo g_shotgun_info = {
	2,								// iSlot
	-1,								// iPosition (-1 = automatic)
	"buckshot",						// pszAmmo1
	(int)gSkillData.sk_ammo_max_buckshot,// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"pizza_ya_san/weapon_as_shotgun",// pszName (path to HUD config)
	AS_SHOTGUN_MAX_CLIP,			// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	AS_SHOTGUN_WEIGHT,				// iWeight
	0,								// iFlagsEx
	15								// accuracy degrees
};

enum ShotgunAnimation
{
	SHOTGUN_IDLE = 0,
	SHOTGUN_FIRE,
	SHOTGUN_FIRE2,
	SHOTGUN_RELOAD,
	SHOTGUN_PUMP,
	SHOTGUN_START_RELOAD,
	SHOTGUN_DRAW,
	SHOTGUN_HOLSTER,
	SHOTGUN_IDLE4,
	SHOTGUN_IDLE_DEEP
};

class CPizzaShotgun : public CWeaponCustom
{
	int m_iShell;

	void Spawn()
	{
		m_iDefaultAmmo = AS_SHOTGUN_DEFAULT_AMMO;
		m_iId = g_shotgun_info.iId;
		CWeaponCustom::Spawn();
	}

	void Precache()
	{
		m_defaultModelV = "models/pizza_ya_san/v_shotgun_shorty.mdl";
		m_defaultModelP = "models/pizza_ya_san/p_shotgun_shorty.mdl";
		m_defaultModelW = "models/pizza_ya_san/w_shotgun_shorty.mdl";
		CBasePlayerWeapon::Precache();

		m_iShell = PRECACHE_MODEL("models/shotgunshell.mdl");// shotgun shell

		PRECACHE_SOUND("items/9mmclip1.wav");

		int shootSnd = PRECACHE_SOUND("weapons/sbarrel1.wav");//shotgun

		int reloadSnd1 = PRECACHE_SOUND("weapons/reload1.wav");	// shotgun reload
		int reloadSnd2 = PRECACHE_SOUND("weapons/reload3.wav");	// shotgun reload

		PRECACHE_SOUND("weapons/sshell1.wav");	// shotgun reload
		PRECACHE_SOUND("weapons/sshell3.wav");	// shotgun reload

		PRECACHE_SOUND("weapons/357_cock1.wav"); // gun empty sound
		int pumpSnd = PRECACHE_SOUND("weapons/scock1.wav");	// cock gun

		PRECACHE_GENERIC("sprites/pizza_ya_san/weapon_as_shotgun.txt");

		animExt = "shotgun";
		//wrongClientWeapon = "weapon_shotgun";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_SHOTGUN_RELOAD;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = SHOTGUN_DRAW;
		params.deployAnimTime = 530;
		params.maxClip = AS_SHOTGUN_MAX_CLIP;
		params.reloadStage[0] = { SHOTGUN_START_RELOAD, 500 }; // 750 = anim time
		params.reloadStage[1] = { SHOTGUN_RELOAD, 250 }; // 480 = anim time
		params.reloadStage[2] = { SHOTGUN_PUMP, 850 };
		params.idles[0] = { SHOTGUN_IDLE, 15, 2330 };
		params.idles[1] = { SHOTGUN_IDLE4, 5, 2330 };
		params.idles[2] = { SHOTGUN_IDLE_DEEP, 80, 4670 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 650;
		primary.accuracyX = 15 * 100;
		primary.accuracyY = 15 * 100;

		float spread = VECTOR_CONE_15DEGREES.x;
		int bulletf = 0;

		AddEvent(WepEvt().Primary().Bullets(AS_SHOTGUN_PELLETCOUNT, 0, gSkillData.sk_12mm_bullet, spread, spread, 0, WC_FLASH_NORMAL, bulletf));
		AddEvent(WepEvt().Primary().WepAnim(SHOTGUN_FIRE));
		AddEvent(WepEvt().Primary().Delay(350).IdleSound(pumpSnd));
		AddEvent(WepEvt().Primary().EjectShell(m_iShell, 14, -12, 6));

		AddEvent(WepEvt().BulletFired().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 94, 109, DISTANT_556, WC_AIVOL_NORMAL));
		AddEvent(WepEvt().BulletFired().PunchSet(-4, 0));

		AddEvent(WepEvt().Reload().Delay(16).IdleSound(reloadSnd1));
		AddEvent(WepEvt().ReloadFinish().Delay(20).IdleSound(pumpSnd));

		PrecacheEvents();
	}

	int GetItemInfo(ItemInfo* p)
	{
		*p = g_shotgun_info;
		return 1;
	}
};

LINK_ENTITY_TO_CLASS(weapon_as_shotgun, CPizzaShotgun);
