#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

enum Mp5Anim
{
	UZI_IDLE,
	UZI_RELOAD,
	UZI_DEPLOY,
	UZI_FIRE1,
	UZI_FIRE2,
	UZI_FIRE3,
};

#define UZI_MAX_CLIP 25

ItemInfo g_uzi_info = {
	2,								// iSlot
	4,								// iPosition (-1 = automatic)
	"9mm",							// pszAmmo1
	gSkillData.sk_ammo_max_9mm,		// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"aomdc/weapon_aom_uzi",				// pszName (path to HUD config)
	UZI_MAX_CLIP,					// iMaxClip
	-1,								// iId (-1 = automatic)
	ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY,	// iFlags
	15,								// iWeight
	0,								// iFlagsEx
	4,								// accuracy degrees
};

class CAomUzi : public CWeaponCustom {

	void Spawn()
	{
		pev->classname = ALLOC_STRING("weapon_aom_uzi");
		m_iDefaultAmmo = UZI_MAX_CLIP;
		m_iId = g_uzi_info.iId;
		CWeaponCustom::Spawn();
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_uzi_info;
		return true;
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_9mmAR"; }

	const char* DisplayName() { return "AOM Uzi"; }

	void Precache() {
		m_defaultModelV = "models/aomdc/v_uzi.mdl";
		m_defaultModelP = "models/aomdc/p_uzi.mdl";
		m_defaultModelW = "models/aomdc/w_uzi.mdl";
		CBasePlayerWeapon::Precache();

		int shootSnd = PRECACHE_SOUND("aomdc/weapons/uzi_fire.wav");
		int reloadSnd1 = PRECACHE_SOUND("aomdc/weapons/uzi_magout.wav");
		int reloadSnd2 = PRECACHE_SOUND("aomdc/weapons/uzi_magin.wav");
		int reloadSnd3 = PRECACHE_SOUND("aomdc/weapons/uzi_boltpull.wav");

		int shell = PRECACHE_MODEL("models/shell.mdl");

		PRECACHE_HUD_FILES("sprites/aomdc/weapon_aom_uzi.txt");

		animExt = "onehanded";
		wrongClientWeapon = "weapon_9mmAR";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_USE_ONLY;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = UZI_DEPLOY;
		params.deployAnimTime = 1100;
		params.maxClip = UZI_MAX_CLIP;
		params.reloadStage[0] = { UZI_RELOAD, 2600 };
		params.idles[0] = { UZI_IDLE, 100, 1000 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 80;
		primary.accuracyX = 4 * 100;
		primary.accuracyY = 4 * 100;

		float spread = VECTOR_CONE_4DEGREES.x;
		int damage = 37;

		AddEvent(WepEvt().Primary().WepAnim(UZI_FIRE1).AddAnim(UZI_FIRE2));
		AddEvent(WepEvt().Primary().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 100, 100, DISTANT_9MM, WC_AIVOL_NORMAL));
		AddEvent(WepEvt().Primary().Bullets(1, 0, damage, spread, spread, 2, WC_FLASH_NORMAL, 0));

		AddEvent(WepEvt().Reload().Delay(510).IdleSound(reloadSnd1));
		AddEvent(WepEvt().Reload().Delay(1290).IdleSound(reloadSnd2));
		AddEvent(WepEvt().Reload().Delay(1910).IdleSound(reloadSnd3));

		AddEvent(WepEvt().Deploy().Delay(450).IdleSound(reloadSnd3));

		AddEvent(WepEvt().BulletFired().EjectShell(shell, 40, -12, 24));
		AddEvent(WepEvt().BulletFired().DLight(20, RGB(255, 255, 255), 1, 0));

		UTIL_PrecacheOther("ammo_uzi");

		PrecacheEvents();
	}

	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
		ammoEntName = "ammo_aom_uzi";
		dropAmount = UZI_MAX_CLIP;
	}
};

#include "CBasePlayerAmmo.h"

class CAmmoAomUzi : public CBasePlayerAmmo
{
	void Spawn(void) {
		Precache();
		SET_MODEL(ENT(pev), "models/aomdc/w_weaponclips/w_uziclip.mdl");
		CBasePlayerAmmo::Spawn();
	}

	void Precache(void) {
		PRECACHE_MODEL("models/aomdc/w_weaponclips/w_uziclip.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther) {
		int bResult = pOther->GiveAmmo(UZI_MAX_CLIP, "9mm", gSkillData.sk_ammo_max_9mm) != -1;
		if (bResult) {
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_uzi, CAmmoAomUzi)
LINK_ENTITY_TO_CLASS(ammo_aom_uzi, CAmmoAomUzi)
LINK_ENTITY_TO_CLASS(weapon_aom_uzi, CAomUzi)