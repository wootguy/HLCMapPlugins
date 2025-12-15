#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

enum DeagleAnim
{
	DEAGLE_IDLE,
	DEAGLE_LONGIDLE,
	DEAGLE_FIRE1,
	DEAGLE_FIRE2,
	DEAGLE_FIRE_LAST,
	DEAGLE_RELOAD,
	DEAGLE_RELOAD_EMPTY,
	DEAGLE_DEPLOY,
};

#define DEAGLE_MAX_CLIP 7

ItemInfo g_deagle_info = {
	3,								// iSlot
	0,								// iPosition (-1 = automatic)
	"357",							// pszAmmo1
	gSkillData.sk_ammo_max_357,		// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"aomdc/weapon_deagle",			// pszName (path to HUD config)
	DEAGLE_MAX_CLIP,				// iMaxClip
	-1,								// iId (-1 = automatic)
	ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY,	// iFlags
	25,								// iWeight
	0,								// iFlagsEx
	3,								// accuracy degrees
};

class CDeagle : public CWeaponCustom {

	void Spawn()
	{
		m_iDefaultAmmo = DEAGLE_MAX_CLIP;
		m_iId = g_deagle_info.iId;
		CWeaponCustom::Spawn();
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_deagle_info;
		return true;
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_357"; }

	const char* DisplayName() { return "AOM Deagle"; }

	void Precache() {
		m_defaultModelV = "models/aomdc/v_deagle.mdl";
		m_defaultModelP = "models/aomdc/p_deagle.mdl";
		m_defaultModelW = "models/aomdc/w_deagle.mdl";
		CBasePlayerWeapon::Precache();

		int shootSnd = PRECACHE_SOUND("aomdc/weapons/deagle_fire.wav");
		int reloadSnd1 = PRECACHE_SOUND("aomdc/weapons/deagle_cout.wav");
		int reloadSnd2 = PRECACHE_SOUND("aomdc/weapons/deagle_cin2.wav");

		int shell = PRECACHE_MODEL("models/shell.mdl");

		PRECACHE_HUD_FILES("sprites/aomdc/weapon_deagle.txt");

		animExt = "python";
		wrongClientWeapon = "weapon_357";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_USE_ONLY;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = DEAGLE_DEPLOY;
		params.deployAnimTime = 620;
		params.maxClip = DEAGLE_MAX_CLIP;
		params.reloadStage[0] = { DEAGLE_RELOAD, 2400 };
		params.reloadStage[1] = { DEAGLE_RELOAD_EMPTY, 2400 };
		params.idles[0] = { DEAGLE_IDLE, 99, 1030 };
		//params.idles[1] = { DEAGLE_LONGIDLE, 1, 1730 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 700;
		primary.accuracyX = 3 * 100;
		primary.accuracyY = 3 * 100;

		float spread = VECTOR_CONE_3DEGREES.x;
		int damage = 150;

		AddEvent(WepEvt().PrimaryNotEmpty().WepAnim(DEAGLE_FIRE1).AddAnim(DEAGLE_FIRE2));
		AddEvent(WepEvt().PrimaryEmpty().WepAnim(DEAGLE_FIRE_LAST));
		AddEvent(WepEvt().Primary().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 100, 100, DISTANT_9MM, WC_AIVOL_NORMAL));
		AddEvent(WepEvt().Primary().Bullets(1, 0, damage, spread, spread, 2, WC_FLASH_NORMAL, 0));
		
		AddEvent(WepEvt().Reload().Delay(260).IdleSound(reloadSnd1));
		AddEvent(WepEvt().Reload().Delay(1560).IdleSound(reloadSnd2));

		AddEvent(WepEvt().BulletFired().PunchAdd(-2.0f, 0));
		AddEvent(WepEvt().BulletFired().EjectShell(shell, 40, -12, 14));
		AddEvent(WepEvt().BulletFired().DLight(20, RGB(255, 255, 255), 1, 0));

		UTIL_PrecacheOther("ammo_deagle");

		PrecacheEvents();
	}

	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
		ammoEntName = "ammo_deagle";
		dropAmount = DEAGLE_MAX_CLIP;
	}
};

#include "CBasePlayerAmmo.h"

class CAmmoDeagle : public CBasePlayerAmmo
{
	void Spawn(void) {
		Precache();
		SET_MODEL(ENT(pev), "models/aomdc/w_weaponclips/w_deagleclip.mdl");
		CBasePlayerAmmo::Spawn();
	}

	void Precache(void) {
		PRECACHE_MODEL("models/aomdc/w_weaponclips/w_deagleclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther) {
		int bResult = pOther->GiveAmmo(DEAGLE_MAX_CLIP, "357", gSkillData.sk_ammo_max_357) != -1;
		if (bResult) {
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_deagle, CAmmoDeagle)
LINK_ENTITY_TO_CLASS(weapon_deagle, CDeagle)