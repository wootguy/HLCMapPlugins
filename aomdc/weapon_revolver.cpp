#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

enum RevolverAnim
{
	REVOLVER_IDLE,
	REVOLVER_FIRE1,
	REVOLVER_FIRE2,
	REVOLVER_FIRE_LAST,
	REVOLVER_RELOAD,
	REVOLVER_DEPLOY,
};

#define REVOLVER_MAX_CLIP 6

ItemInfo g_revolver_info = {
	3,								// iSlot
	0,								// iPosition (-1 = automatic)
	"357",							// pszAmmo1
	gSkillData.sk_ammo_max_357,		// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"aomdc/weapon_revolver",		// pszName (path to HUD config)
	REVOLVER_MAX_CLIP,				// iMaxClip
	-1,								// iId (-1 = automatic)
	ITEM_FLAG_NOAUTOSWITCHEMPTY | ITEM_FLAG_SELECTONEMPTY,	// iFlags
	25,								// iWeight
	0,								// iFlagsEx
	3,								// accuracy degrees
};

class CRevolver : public CWeaponCustom {

	void Spawn()
	{
		m_iDefaultAmmo = REVOLVER_MAX_CLIP;
		m_iId = g_revolver_info.iId;
		CWeaponCustom::Spawn();
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_revolver_info;
		return true;
	}

	const char* DisplayName() { return "AOM Revolver"; }

	void Precache() {
		m_defaultModelV = "models/aomdc/v_revolver.mdl";
		m_defaultModelP = "models/aomdc/p_revolver.mdl";
		m_defaultModelW = "models/aomdc/w_revolver.mdl";
		CBasePlayerWeapon::Precache();

		int shootSnd = PRECACHE_SOUND("aomdc/weapons/revolver_fire.wav");
		int reloadSnd = PRECACHE_SOUND("aomdc/weapons/bull_reload.wav");
		int drawSnd = PRECACHE_SOUND("aomdc/weapons/bull_draw.wav");

		int shell = PRECACHE_MODEL("models/shell.mdl");

		PRECACHE_HUD_FILES("sprites/aomdc/weapon_revolver.txt");

		animExt = "python";
		wrongClientWeapon = "weapon_357";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_USE_ONLY;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = REVOLVER_DEPLOY;
		params.deployAnimTime = 1280;
		params.maxClip = REVOLVER_MAX_CLIP;
		params.reloadStage[0] = { REVOLVER_RELOAD, 2760 };
		params.idles[0] = { REVOLVER_IDLE, 99, 1030 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 900;
		primary.accuracyX = 3 * 100;
		primary.accuracyY = 3 * 100;

		float spread = VECTOR_CONE_3DEGREES.x;
		int damage = 200;

		AddEvent(WepEvt().PrimaryNotEmpty().WepAnim(REVOLVER_FIRE1).AddAnim(REVOLVER_FIRE2));
		AddEvent(WepEvt().PrimaryEmpty().WepAnim(REVOLVER_FIRE_LAST));
		AddEvent(WepEvt().Primary().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 100, 100, DISTANT_9MM, WC_AIVOL_NORMAL));
		AddEvent(WepEvt().Primary().Bullets(1, 0, damage, spread, spread, 2, WC_FLASH_NORMAL, 0));

		AddEvent(WepEvt().Reload().IdleSound(reloadSnd));
		AddEvent(WepEvt().Deploy().IdleSound(drawSnd));

		AddEvent(WepEvt().BulletFired().PunchAdd(-2.0f, 0));
		AddEvent(WepEvt().BulletFired().DLight(20, RGB(255, 255, 255), 1, 0));

		UTIL_PrecacheOther("ammo_revolver");

		PrecacheEvents();
	}

	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
		ammoEntName = "ammo_revolver";
		dropAmount = REVOLVER_MAX_CLIP;
	}
};

#include "CBasePlayerAmmo.h"

class CAmmoRevolver : public CBasePlayerAmmo
{
	void Spawn(void) {
		Precache();
		SET_MODEL(ENT(pev), "models/aomdc/w_weaponclips/w_revolverrounds.mdl");
		CBasePlayerAmmo::Spawn();
	}

	void Precache(void) {
		PRECACHE_MODEL("models/aomdc/w_weaponclips/w_revolverrounds.mdl");
		PRECACHE_SOUND("items/9mmclip1.wav");
	}

	BOOL AddAmmo(CBaseEntity* pOther) {
		int bResult = pOther->GiveAmmo(REVOLVER_MAX_CLIP, "357", gSkillData.sk_ammo_max_357) != -1;
		if (bResult) {
			EMIT_SOUND(ENT(pev), CHAN_ITEM, "items/9mmclip1.wav", 1, ATTN_NORM);
		}
		return bResult;
	}
};

LINK_ENTITY_TO_CLASS(ammo_revolver, CAmmoRevolver)
LINK_ENTITY_TO_CLASS(weapon_revolver, CRevolver)