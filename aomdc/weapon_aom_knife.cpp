#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

enum KnifeAnim
{
	KNIFE_IDLE,
	KNIFE_DRAW,
	KNIFE_HOLSTER,
	KNIFE_SLASH,
	KNIFE_SLASH2,
};

ItemInfo g_knife_info = {
	0,								// iSlot
	0,								// iPosition (-1 = automatic)
	NULL,							// pszAmmo1
	-1,								// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"aomdc/weapon_aom_knife",		// pszName (path to HUD config)
	-1,								// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	5,								// iWeight
};

class CKnife : public CWeaponCustom {

	void Spawn()
	{
		pev->classname = ALLOC_STRING("weapon_aom_knife");
		m_iId = g_knife_info.iId;
		CWeaponCustom::Spawn();
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_knife_info;
		return true;
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_crowbar"; }

	void Precache() {
		m_defaultModelV = "models/aomdc/v_kitchenknife.mdl";
		m_defaultModelP = "models/aomdc/p_kitchenknife.mdl";
		m_defaultModelW = "models/aomdc/w_kitchenknife.mdl";
		CBasePlayerWeapon::Precache();

		PRECACHE_HUD_FILES("sprites/aomdc/weapon_aom_knife.txt");

		animExt = "crowbar";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_USE_ONLY | FL_WC_WEP_NO_PREDICTION;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = KNIFE_DRAW;
		params.deployAnimTime = 640;
		params.idles[0] = { KNIFE_IDLE, 100, 4000 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.flags = FL_WC_SHOOT_UNDERWATER | FL_WC_SHOOT_IS_MELEE;
		primary.melee.damage = 10;
		primary.melee.damageBits = DMG_SLASH | DMG_NEVERGIB; // less damage against breakables
		primary.melee.range = 32;
		primary.melee.missCooldown = 500;
		primary.melee.hitCooldown = 250;
		primary.melee.decalDelay = 200;
		primary.melee.missSounds[0] = PRECACHE_SOUND("aomdc/weapons/knife_swing1.wav");
		primary.melee.hitWallSounds[0] = PRECACHE_SOUND("aomdc/weapons/knife_wall1.wav");
		primary.melee.hitWallSounds[1] = PRECACHE_SOUND("aomdc/weapons/knife_wall2.wav");
		primary.melee.hitFleshSounds[0] = PRECACHE_SOUND("aomdc/weapons/knife_hit1.wav");
		primary.melee.hitFleshSounds[1] = PRECACHE_SOUND("aomdc/weapons/knife_hit2.wav");

		AddEvent(WepEvt().Primary().WepAnim(KNIFE_SLASH, 0, FL_WC_ANIM_ORDERED).AddAnim(KNIFE_SLASH2));

		PrecacheEvents();
	}
};

LINK_ENTITY_TO_CLASS(weapon_aom_knife, CKnife)