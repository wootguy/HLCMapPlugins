#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

enum AxeAnim
{
	AXE_IDLE,
	AXE_SLASH,
	AXE_DRAW,
};

ItemInfo g_axe_info = {
	0,								// iSlot
	0,								// iPosition (-1 = automatic)
	NULL,							// pszAmmo1
	-1,								// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"aomdc/weapon_axe",				// pszName (path to HUD config)
	-1,								// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	5,								// iWeight
};

class CAxe : public CWeaponCustom {

	void Spawn()
	{
		m_iId = g_axe_info.iId;
		CWeaponCustom::Spawn();
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_axe_info;
		return true;
	}

	void Precache() {
		m_defaultModelV = "models/aomdc/v_axe.mdl";
		m_defaultModelP = "models/aomdc/p_axe.mdl";
		m_defaultModelW = "models/aomdc/w_axe.mdl";
		CBasePlayerWeapon::Precache();

		PRECACHE_HUD_FILES("sprites/aomdc/weapon_axe.txt");

		animExt = "crowbar";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_USE_ONLY | FL_WC_WEP_NO_PREDICTION;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = AXE_DRAW;
		params.deployAnimTime = 1400;
		params.idles[0] = { AXE_IDLE, 100, 1000 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.flags = FL_WC_SHOOT_UNDERWATER | FL_WC_SHOOT_IS_MELEE;
		primary.melee.damage = 35;
		primary.melee.range = 32;
		primary.melee.missCooldown = 1000;
		primary.melee.hitCooldown = 1000;
		primary.melee.decalDelay = 200;
		primary.melee.damageBits = DMG_SLASH | DMG_NEVERGIB;
		primary.melee.missSounds[0] = PRECACHE_SOUND("aomdc/weapons/axe_swing.wav");
		primary.melee.hitWallSounds[0] = PRECACHE_SOUND("aomdc/weapons/axe_hit.wav");
		primary.melee.hitFleshSounds[0] = PRECACHE_SOUND("aomdc/weapons/axe_hitbody.wav");

		AddEvent(WepEvt().Primary().WepAnim(AXE_SLASH));

		PrecacheEvents();
	}
};

LINK_ENTITY_TO_CLASS(weapon_axe, CAxe)