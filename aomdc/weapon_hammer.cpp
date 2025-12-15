#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

enum HammerAnim
{
	HAMMER_IDLE,
	HAMMER_WHACK,
	HAMMER_DRAW,
};

ItemInfo g_hammer_info = {
	0,								// iSlot
	0,								// iPosition (-1 = automatic)
	NULL,							// pszAmmo1
	-1,								// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"aomdc/weapon_hammer",			// pszName (path to HUD config)
	-1,								// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	5,								// iWeight
};

class CHammer : public CWeaponCustom {

	void Spawn()
	{
		m_iId = g_hammer_info.iId;
		CWeaponCustom::Spawn();
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_hammer_info;
		return true;
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	void Precache() {
		m_defaultModelV = "models/aomdc/v_hammer.mdl";
		m_defaultModelP = "models/aomdc/p_hammer.mdl";
		m_defaultModelW = "models/aomdc/w_hammer.mdl";
		CBasePlayerWeapon::Precache();

		PRECACHE_HUD_FILES("sprites/aomdc/weapon_hammer.txt");

		animExt = "crowbar";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_USE_ONLY | FL_WC_WEP_NO_PREDICTION;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = HAMMER_DRAW;
		params.deployAnimTime = 700;
		params.idles[0] = { HAMMER_IDLE, 100, 1000 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.flags = FL_WC_SHOOT_UNDERWATER | FL_WC_SHOOT_IS_MELEE | FL_WC_SHOOT_CHARGEUP_ONCE;
		primary.melee.damage = 40;
		primary.melee.range = 32;
		primary.chargeTime = 500;
		primary.chargeCancelTime = 500;
		primary.melee.missCooldown = 1000;
		primary.melee.hitCooldown = 1000;
		primary.melee.decalDelay = 0;
		primary.melee.damageBits = DMG_SLASH | DMG_NEVERGIB;
		primary.melee.missSounds[0] = PRECACHE_SOUND("aomdc/weapons/hammer_swing.wav");
		primary.melee.hitWallSounds[0] = PRECACHE_SOUND("aomdc/weapons/hammer_hit.wav");
		primary.melee.hitFleshSounds[0] = PRECACHE_SOUND("aomdc/weapons/hammer_hitbody.wav");

		//AddEvent(WepEvt().Primary().WepAnim(HAMMER_WHACK));
		AddEvent(WepEvt().PrimaryCharge().WepAnim(HAMMER_WHACK));

		PrecacheEvents();
	}

	bool MeleeHit(CBasePlayer* plr, CBaseEntity* target) override {
		UTIL_ScreenShake(plr, 6, 5, 0.7f);
		return false;
	}
};

LINK_ENTITY_TO_CLASS(weapon_hammer, CHammer)