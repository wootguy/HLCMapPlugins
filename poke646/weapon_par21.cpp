#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

const float PAR_MOD_DAMAGE = 15.0;
const float PAR_MOD_FIRERATE = 0.10;

enum PARAnimation
{
	PAR_LONGIDLE = 0,
	PAR_IDLE1,
	PAR_LAUNCH,
	PAR_RELOAD,
	PAR_DEPLOY,
	PAR_FIRE1,
	PAR_FIRE2,
	PAR_FIRE3,
};

const int PAR_DEFAULT_GIVE = 30;
const int PAR_MAX_AMMO = 150;
const int PAR_MAX_AMMO2 = 10;
const int PAR_MAX_CLIP = 30;
const int PAR_WEIGHT = 5;

ItemInfo g_par21_info = {
	2,								// iSlot
	-1,								// iPosition (-1 = automatic)
	"9mm",							// pszAmmo1
	PAR_MAX_AMMO,					// iMaxAmmo1
	"ARgrenades",					// pszAmmo2
	PAR_MAX_AMMO2,					// iMaxAmmo2
	"poke646/weapon_par21",			// pszName (path to HUD config)
	PAR_MAX_CLIP,					// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	PAR_WEIGHT,						// iWeight
	0,								// iFlagsEx
	3,								// accuracy degrees
};

class CPar21 : public CWeaponCustom {

	void Spawn()
	{
		m_iDefaultAmmo = PAR_DEFAULT_GIVE;
		m_iId = g_par21_info.iId;
		CWeaponCustom::Spawn();
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_9mmAR"; }

	void Precache() {
		m_defaultModelV = "models/vendetta/weapons/par21/v_par21.mdl";
		m_defaultModelP = "models/vendetta/weapons/par21/p_par21.mdl";
		m_defaultModelW = "models/vendetta/weapons/par21/w_par21.mdl";
		CBasePlayerWeapon::Precache();

		int shootSnd = PRECACHE_SOUND("vendetta/weapons/par21/par21_fire.wav");
		int launchSnd1 = PRECACHE_SOUND("vendetta/weapons/par21/par21_gl1.wav");
		int launchSnd2 = PRECACHE_SOUND("vendetta/weapons/par21/par21_gl2.wav");
		int reloadSnd = PRECACHE_SOUND("vendetta/weapons/par21/par21_reload.wav");

		int shell = PRECACHE_MODEL("models/shell.mdl");

		PRECACHE_HUD_FILES("sprites/poke646/weapon_par21.txt");

		animExt = "mp5";
		wrongClientWeapon = "weapon_9mmAR";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_HAS_SECONDARY;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = PAR_DEPLOY;
		params.deployAnimTime = 1000;
		params.maxClip = PAR_MAX_CLIP;
		params.reloadStage[0] = { PAR_RELOAD, 1570 };
		params.idles[0] = { PAR_LONGIDLE, 20, 5130 };
		params.idles[1] = { PAR_IDLE1, 80, 3170 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 100;
		primary.accuracyX = 3 * 100;
		primary.accuracyY = 3 * 100;

		CustomWeaponShootOpts& secondary = params.shootOpts[1];
		secondary.ammoCost = 1;
		secondary.cooldown = 1000;
		secondary.accuracyX = 3 * 100;
		secondary.accuracyY = 3 * 100;

		float spread = VECTOR_CONE_3DEGREES.x;

		AddEvent(WepEvt().Primary().WepAnim(PAR_FIRE1).AddAnim(PAR_FIRE2).AddAnim(PAR_FIRE3));
		AddEvent(WepEvt().Primary().PlaySound(shootSnd, CHAN_WEAPON, 1.0f, ATTN_NORM, 94, 109, DISTANT_9MM, WC_AIVOL_NORMAL));
		AddEvent(WepEvt().Primary().Bullets(1, 0, 15, spread, spread, 2, WC_FLASH_NORMAL, 0));
		
		AddEvent(WepEvt().BulletFired().PunchRandom(1.5f, 0));
		AddEvent(WepEvt().BulletFired().EjectShell(shell, 20, -12, 14));

		AddEvent(WepEvt().Secondary().WepAnim(PAR_LAUNCH));
		AddEvent(WepEvt().Secondary().PunchSet(-10, 0));
		AddEvent(WepEvt().Secondary()
			.PlaySound(launchSnd1, CHAN_WEAPON, 1.0f, ATTN_NORM, 94, 109, DISTANT_556, WC_AIVOL_LOUD)
			.AddSound(launchSnd2));
		AddEvent(WepEvt().Secondary()
			.Projectile(WC_PROJECTILE_ARGRENADE, 800)
			.ProjPhysics(0.5f));

		AddEvent(WepEvt().Reload().Delay(330).IdleSound(reloadSnd));

		PrecacheEvents();
	}

	int GetItemInfo(ItemInfo* info) {
		*info = g_par21_info;
		return true;
	}

	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
		if (secondary) {
			ammoEntName = "ammo_ARgrenades";
			dropAmount = 2;
		}
		else {
			ammoEntName = "ammo_9mmAR";
			dropAmount = PAR_MAX_CLIP;
		}
	}
};

LINK_ENTITY_TO_CLASS(weapon_par21, CPar21);