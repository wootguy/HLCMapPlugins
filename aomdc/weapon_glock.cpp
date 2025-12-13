#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

enum GlockAnim
{
	GLOCK_IDLE,
	GLOCK_LONGIDLE,
	GLOCK_FIRE1,
	GLOCK_FIRE2,
	GLOCK_FIRE_LAST,
	GLOCK_RELOAD,
	GLOCK_RELOAD_EMPTY,
	GLOCK_DEPLOY,
};

#define GLOCK_MAX_CLIP 20

ItemInfo g_glock_info = {
	1,								// iSlot
	2,								// iPosition (-1 = automatic)
	"9mm",							// pszAmmo1
	gSkillData.sk_ammo_max_9mm,		// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"aomdc/weapon_aom_glock",		// pszName (path to HUD config)
	GLOCK_MAX_CLIP,					// iMaxClip
	-1,								// iId (-1 = automatic)
	ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY,	// iFlags
	5,								// iWeight
	0,								// iFlagsEx
	2,								// accuracy degrees
};

class CGlock : public CWeaponCustom {

	void Spawn()
	{
		m_iDefaultAmmo = GLOCK_MAX_CLIP;
		m_iId = g_glock_info.iId;
		CWeaponCustom::Spawn();
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_glock_info;
		return true;
	}

	const char* DisplayName() { return "AOM Glock"; }

	void Precache() {
		m_defaultModelV = "models/aomdc/v_glock.mdl";
		m_defaultModelP = "models/aomdc/p_glock.mdl";
		m_defaultModelW = "models/aomdc/w_glock.mdl";
		CBasePlayerWeapon::Precache();

		int shootSnd = PRECACHE_SOUND("aomdc/weapons/glock_fire.wav");
		int reloadSnd1 = PRECACHE_SOUND("aomdc/weapons/p228_magout.wav");
		int reloadSnd2 = PRECACHE_SOUND("aomdc/weapons/ber_magplace.wav");
		int reloadSnd3 = PRECACHE_SOUND("aomdc/weapons/p228_magin.wav");
		int reloadSnd4 = PRECACHE_SOUND("aomdc/weapons/p228_slideforward.wav");

		int shell = PRECACHE_MODEL("models/shell.mdl");

		PRECACHE_HUD_FILES("sprites/aomdc/weapon_p228.txt");

		animExt = "onehanded";
		wrongClientWeapon = "weapon_9mmhandgun";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_USE_ONLY;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = GLOCK_DEPLOY;
		params.deployAnimTime = 520;
		params.maxClip = GLOCK_MAX_CLIP;
		params.reloadStage[0] = { GLOCK_RELOAD, 2100 };
		params.reloadStage[1] = { GLOCK_RELOAD_EMPTY, 2100 };
		params.idles[0] = { GLOCK_IDLE, 99, 1030 };
		//params.idles[1] = { GLOCK_LONGIDLE, 1, 1730 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 50;
		primary.accuracyX = 2 * 100;
		primary.accuracyY = 2 * 100;
		primary.flags = FL_WC_SHOOT_NO_AUTOFIRE;

		float spread = VECTOR_CONE_2DEGREES.x;
		int damage = 17;

		AddEvent(WepEvt().PrimaryNotEmpty().WepAnim(GLOCK_FIRE1).AddAnim(GLOCK_FIRE2));
		AddEvent(WepEvt().PrimaryEmpty().WepAnim(GLOCK_FIRE_LAST));
		AddEvent(WepEvt().Primary().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 100, 100, DISTANT_9MM, WC_AIVOL_NORMAL));
		AddEvent(WepEvt().Primary().Bullets(1, 0, damage, spread, spread, 2, WC_FLASH_NORMAL, 0));
		
		AddEvent(WepEvt().ReloadNotEmpty().Delay(200).IdleSound(reloadSnd1));
		AddEvent(WepEvt().ReloadNotEmpty().Delay(1500).IdleSound(reloadSnd2));
		AddEvent(WepEvt().ReloadNotEmpty().Delay(1666).IdleSound(reloadSnd3));

		AddEvent(WepEvt().ReloadEmpty().Delay(360).IdleSound(reloadSnd1));
		AddEvent(WepEvt().ReloadEmpty().Delay(960).IdleSound(reloadSnd2));
		AddEvent(WepEvt().ReloadEmpty().Delay(1130).IdleSound(reloadSnd3));
		AddEvent(WepEvt().ReloadEmpty().Delay(1530).IdleSound(reloadSnd4));

		AddEvent(WepEvt().BulletFired().PunchAdd(-2.0f, 0));
		AddEvent(WepEvt().BulletFired().EjectShell(shell, 40, -12, 14));
		AddEvent(WepEvt().BulletFired().DLight(20, RGB(255, 255, 255), 1, 0));

		UTIL_PrecacheOther("ammo_glock");

		PrecacheEvents();
	}

	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
		ammoEntName = "ammo_glock";
		dropAmount = GLOCK_MAX_CLIP;
	}
};

#include "CBasePlayerAmmo.h"

class CAmmoGlock : public CBasePlayerAmmo
{
	void Spawn(void) {
		Precache();
		SET_MODEL(ENT(pev), "models/aomdc/w_weaponclips/w_glockclip.mdl");
		CBasePlayerAmmo::Spawn();
	}

	void Precache(void) {
		PRECACHE_MODEL("models/aomdc/w_weaponclips/w_glockclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther) {
		int bResult = pOther->GiveAmmo(GLOCK_MAX_CLIP, "9mm", gSkillData.sk_ammo_max_9mm) != -1;
		if (bResult) {
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_glock, CAmmoGlock)
LINK_ENTITY_TO_CLASS(weapon_aom_glock, CGlock)