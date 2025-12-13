#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

const float BRADNAILER_DAMAGE = 15.0;
const float BRADNAILER_ALT_DAMAGE = 15.0;
const float BRADNAILER_MOD_PRIM_FIRERATE = 0.30;
const float BRADNAILER_MOD_ALT_FIRERATE = 0.20;

const int BRADNAILER_DEFAULT_AMMO = 25;
const int BRADNAILER_MAX_CARRY = 200;
const int BRADNAILER_MAX_CLIP = 25;
const int BRADNAILER_WEIGHT = 5;

enum BRADNAILERAnimation
{
	BRADNAILER_IDLE1 = 0,
	BRADNAILER_IDLE2,
	BRADNAILER_IDLE3,
	BRADNAILER_SHOOT,
	BRADNAILER_SHOOT_EMPTY,
	BRADNAILER_RELOAD,
	BRADNAILER_RELOAD_NOSHOT,
	BRADNAILER_DRAW,
	BRADNAILER_HOLSTER,
	BRADNAILER_ADD_SILENCER,
	BRADNAILER_TILT_DOWN,
	BRADNAILER_TILT_UP,
	BRADNAILER_FAST_SHOOT
};

ItemInfo g_bradnailer_info = {
	1,								// iSlot
	-1,								// iPosition (-1 = automatic)
	"9mm",							// pszAmmo1
	BRADNAILER_MAX_CARRY,			// iMaxAmmo1
	NULL,							// pszAmmo2
	-1,								// iMaxAmmo2
	"poke646/weapon_bradnailer",	// pszName (path to HUD config)
	BRADNAILER_MAX_CLIP,			// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	BRADNAILER_WEIGHT,				// iWeight
	0,								// iFlagsEx
	3,								// accuracy degrees
	6,								// accuracy degrees
};

class CBradNailer : public CWeaponCustom {

	void Spawn()
	{
		m_iDefaultAmmo = BRADNAILER_DEFAULT_AMMO;
		m_iId = g_bradnailer_info.iId;
		CWeaponCustom::Spawn();
	}

	void Precache() {
		m_defaultModelV = "models/poke646/weapons/bradnailer/v_bradnailer.mdl";
		m_defaultModelP = "models/poke646/weapons/bradnailer/p_bradnailer.mdl";
		m_defaultModelW = "models/poke646/weapons/bradnailer/w_bradnailer.mdl";
		CBasePlayerWeapon::Precache();

		int shootSnd = PRECACHE_SOUND("poke646/weapons/bradnailer/bradnailer_fire.wav");
		int nailMdl = PRECACHE_MODEL("models/poke646/weapons/nail.mdl");
		int reloadSnd1 = PRECACHE_SOUND("items/9mmclip2.wav");
		int reloadSnd2 = PRECACHE_SOUND("items/9mmclip1.wav");

		PRECACHE_HUD_FILES("sprites/poke646/weapon_bradnailer.txt");

		animExt = "mp5";
		//wrongClientWeapon = "weapon_mp5";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_HAS_SECONDARY;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = BRADNAILER_DRAW;
		params.deployAnimTime = 1130;
		params.maxClip = BRADNAILER_MAX_CLIP;
		params.reloadStage[0] = { BRADNAILER_RELOAD, 2390 };
		params.idles[0] = { BRADNAILER_IDLE1, 20, 3940 };
		params.idles[1] = { BRADNAILER_IDLE2, 20, 2690 };
		params.idles[2] = { BRADNAILER_IDLE3, 60, 3710 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 300;
		primary.accuracyX = 1 * 100;
		primary.accuracyY = 1 * 100;
		primary.flags = FL_WC_SHOOT_UNDERWATER;

		CustomWeaponShootOpts& secondary = params.shootOpts[1];
		secondary.ammoCost = 1;
		secondary.ammoPool = WC_AMMOPOOL_PRIMARY_CLIP;
		secondary.cooldown = 200;
		secondary.chargeTime = 450;
		secondary.chargeCancelTime = 450;
		secondary.accuracyX = 6 * 100;
		secondary.accuracyY = 6 * 100;
		secondary.flags = FL_WC_SHOOT_UNDERWATER;

		float spread = VECTOR_CONE_1DEGREES.x;
		float spread2 = VECTOR_CONE_6DEGREES.x;
		int bulletf = FL_WC_BULLETS_NO_DECAL;

		AddEvent(WepEvt().Primary().WepAnim(BRADNAILER_SHOOT));
		AddEvent(WepEvt().Primary().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 94, 109, DISTANT_NONE, WC_AIVOL_NORMAL));
		AddEvent(WepEvt().Primary().PunchSet(-1.5f, 0));
		AddEvent(WepEvt().Primary().Projectile(WC_PROJECTILE_CUSTOM, 4000, spread, spread, Vector(8, 16, -6))
			.ProjClass(ALLOC_STRING("nail"))
			.ProjModel(nailMdl));

		AddEvent(WepEvt().SecondaryCharge().WepAnim(BRADNAILER_TILT_DOWN));
		AddEvent(WepEvt().SecondaryStop().WepAnim(BRADNAILER_TILT_UP));
		AddEvent(WepEvt().SecondaryStop().Cooldown(450, FL_WC_COOLDOWN_IDLE | FL_WC_COOLDOWN_PRIMARY | FL_WC_COOLDOWN_SECONDARY));
		AddEvent(WepEvt().Secondary().WepAnim(BRADNAILER_FAST_SHOOT));
		AddEvent(WepEvt().Secondary().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 94, 109, DISTANT_NONE, WC_AIVOL_QUIET));
		AddEvent(WepEvt().Secondary().PunchSet(-1.5f, 0));
		AddEvent(WepEvt().Secondary().Projectile(WC_PROJECTILE_CUSTOM, 4000, spread2, spread2, Vector(0, 16, -6))
			.ProjClass(ALLOC_STRING("nail"))
			.ProjModel(nailMdl));

		AddEvent(WepEvt().Reload().Delay(220).IdleSound(reloadSnd1));
		AddEvent(WepEvt().Reload().Delay(1280).IdleSound(reloadSnd2));

		PrecacheEvents();
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_bradnailer_info;
		return true;
	}
};

LINK_ENTITY_TO_CLASS(weapon_bradnailer, CBradNailer);