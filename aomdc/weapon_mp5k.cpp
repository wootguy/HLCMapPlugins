#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

enum Mp5Anim
{
	MP5K_IDLE,
	MP5K_FIDGET,
	MP5K_FIRE1,
	MP5K_FIRE2,
	MP5K_RELOAD,
	MP5K_DEPLOY,
};

#define MP5K_MAX_CLIP 30

ItemInfo g_mp5k_info = {
	2,								// iSlot
	4,								// iPosition (-1 = automatic)
	"9mm",							// pszAmmo1
	gSkillData.sk_ammo_max_9mm,		// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"aomdc/weapon_mp5k",			// pszName (path to HUD config)
	MP5K_MAX_CLIP,					// iMaxClip
	-1,								// iId (-1 = automatic)
	ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY,	// iFlags
	15,								// iWeight
	0,								// iFlagsEx
	4,								// accuracy degrees
};

class CMp5k : public CWeaponCustom {

	void Spawn()
	{
		m_iDefaultAmmo = MP5K_MAX_CLIP;
		m_iId = g_mp5k_info.iId;
		CWeaponCustom::Spawn();
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_mp5k_info;
		return true;
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_9mmAR"; }

	const char* DisplayName() { return "AOM MP5K"; }

	void Precache() {
		m_defaultModelV = "models/aomdc/v_mp5k.mdl";
		m_defaultModelP = "models/aomdc/p_mp5k.mdl";
		m_defaultModelW = "models/aomdc/w_mp5k.mdl";
		CBasePlayerWeapon::Precache();

		int shootSnd = PRECACHE_SOUND("aomdc/weapons/mp5k_fire.wav");
		int reloadSnd1 = PRECACHE_SOUND("aomdc/weapons/mp5k_boltback.wav");
		int reloadSnd2 = PRECACHE_SOUND("aomdc/weapons/mp5k_magout.wav");
		int reloadSnd3 = PRECACHE_SOUND("aomdc/weapons/mp5k_magin.wav");
		int reloadSnd4 = PRECACHE_SOUND("aomdc/weapons/mp5k_boltforward.wav");

		int shell = PRECACHE_MODEL("models/shell.mdl");

		PRECACHE_HUD_FILES("sprites/aomdc/weapon_mp5k.txt");

		animExt = "mp5";
		wrongClientWeapon = "weapon_9mmAR";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_USE_ONLY;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = MP5K_DEPLOY;
		params.deployAnimTime = 1030;
		params.maxClip = MP5K_MAX_CLIP;
		params.reloadStage[0] = { MP5K_RELOAD, 3370 };
		params.idles[0] = { MP5K_IDLE, 90, 1030 };
		params.idles[1] = { MP5K_FIDGET, 10, 2700 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 60;
		primary.accuracyX = 4 * 100;
		primary.accuracyY = 4 * 100;

		float spread = VECTOR_CONE_4DEGREES.x;
		int damage = 31;

		AddEvent(WepEvt().Primary().WepAnim(MP5K_FIRE1).AddAnim(MP5K_FIRE2));
		AddEvent(WepEvt().Primary().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 100, 100, DISTANT_9MM, WC_AIVOL_NORMAL));
		AddEvent(WepEvt().Primary().Bullets(1, 0, damage, spread, spread, 2, WC_FLASH_NORMAL, 0));

		AddEvent(WepEvt().Reload().Delay(260).IdleSound(reloadSnd1));
		AddEvent(WepEvt().Reload().Delay(1100).IdleSound(reloadSnd2));
		AddEvent(WepEvt().Reload().Delay(1770).IdleSound(reloadSnd3));
		AddEvent(WepEvt().Reload().Delay(2300).IdleSound(reloadSnd4));

		AddEvent(WepEvt().BulletFired().EjectShell(shell, 40, -12, 34));
		AddEvent(WepEvt().BulletFired().DLight(20, RGB(255, 255, 255), 1, 0));

		UTIL_PrecacheOther("ammo_mp5k");

		PrecacheEvents();
	}

	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
		ammoEntName = "ammo_mp5k";
		dropAmount = MP5K_MAX_CLIP;
	}
};

#include "CBasePlayerAmmo.h"

class CAmmoMp5k : public CBasePlayerAmmo
{
	void Spawn(void) {
		Precache();
		SET_MODEL(ENT(pev), "models/aomdc/w_weaponclips/w_mp5kclip.mdl");
		CBasePlayerAmmo::Spawn();
	}

	void Precache(void) {
		PRECACHE_MODEL("models/aomdc/w_weaponclips/w_mp5kclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther) {
		int bResult = pOther->GiveAmmo(MP5K_MAX_CLIP, "9mm", gSkillData.sk_ammo_max_9mm) != -1;
		if (bResult) {
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_mp5k, CAmmoMp5k)
LINK_ENTITY_TO_CLASS(weapon_mp5k, CMp5k)