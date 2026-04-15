#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

enum USPAnimation
{
	USP_IDLE = 0,
	USP_SHOOT1,
	USP_SHOOT2,
	USP_SHOOT3,
	USP_SHOOTLAST,
	USP_RELOAD,
	USP_DRAW,
	USP_ADD_SILENCER,
	USP_IDLE_UNSIL,
	USP_SHOOT1_UNSIL,
	USP_SHOOT2_UNSIL,
	USP_SHOOT3_UNSIL,
	USP_SHOOTLAST_UNSIL,
	USP_RELOAD_UNSIL,
	USP_DRAW_UNSIL,
	USP_DETACH_SILENCER
};

const int USP_DEFAULT_GIVE = 24;
const int USP_MAX_CARRY = 100;
const int USP_MAX_CLIP = 12;
const int USP_WEIGHT = 5;

ItemInfo g_usp_info = {
	1,								// iSlot
	-1,								// iPosition (-1 = automatic)
	"9mm",							// pszAmmo1
	NULL,							// pszAmmo2
	"kuilu/weapon_usp",				// pszName (path to HUD config)
	USP_MAX_CLIP,					// iMaxClip
	-1,								// iId (-1 = automatic)
	0,								// iFlags
	USP_WEIGHT,						// iWeight
	0,								// iFlagsEx
	2								// accuracy degrees
};

class CUsp : public CWeaponCustom {
	bool m_silencer;

	CustomWeaponParams unsilenced_params;
	CustomWeaponParams silenced_params;

	int GetItemInfo(ItemInfo* info) {
		*info = g_usp_info;
		return true;
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_9mmhandgun"; }

	const char* DisplayName() { return "USP"; }

	void Precache() {
		m_defaultModelV = "models/rngstuff/kuilu/weapons/v_p228.mdl";
		m_defaultModelP = "models/rngstuff/kuilu/weapons/p_p228.mdl";
		m_defaultModelW = "models/rngstuff/kuilu/weapons/w_p228.mdl";
		CBasePlayerWeapon::Precache();

		int shootSnd = PRECACHE_SOUND("rng/kuilu/weapons/p228_fire.wav");
		int shootSnd_alt = PRECACHE_SOUND("rng/kuilu/weapons/p228_fire_sil.wav");
		int reloadSnd1 = PRECACHE_SOUND("rng/kuilu/weapons/p228_clipout.wav");
		int reloadSnd2 = PRECACHE_SOUND("rng/kuilu/weapons/p228_clipin.wav");
		int reloadSnd3 = PRECACHE_SOUND("rng/kuilu/weapons/p228_slide.wav");
		int reloadSnd3_alt = PRECACHE_SOUND("rng/kuilu/weapons/p228_slide_sil.wav");
		int deploySnd = PRECACHE_SOUND("rng/kuilu/weapons/p228_draw.wav");
		int deploySnd2 = PRECACHE_SOUND("rng/kuilu/weapons/p228_slidepull.wav");
		int deploySnd2_alt = PRECACHE_SOUND("rng/kuilu/weapons/p228_slidepull_sil.wav");
		int silSnd = PRECACHE_SOUND("rng/kuilu/weapons/p228_silencer_on.wav");
		int silSnd_alt = PRECACHE_SOUND("rng/kuilu/weapons/p228_silencer_off.wav");

		int shell = PRECACHE_MODEL("models/shell.mdl");

		PRECACHE_HUD_FILES("sprites/kuilu/weapon_usp.txt");

		animExt = "onehanded";
		wrongClientWeapon = "weapon_9mmhandgun";

		params.flags = FL_WC_WEP_HAS_PRIMARY | FL_WC_WEP_HAS_SECONDARY;
		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = USP_DRAW_UNSIL;
		params.deployTime = 1000;
		params.deployAnimTime = 1040;
		params.maxClip = USP_MAX_CLIP;
		params.reloadStage[0] = { USP_RELOAD_UNSIL, 2730 };
		params.idles[0] = { USP_IDLE_UNSIL, 99, 2000 };

		CustomWeaponShootOpts& primary = params.shootOpts[0];
		primary.ammoCost = 1;
		primary.cooldown = 217;
		primary.accuracyX = 2 * 100;
		primary.accuracyY = 2 * 100;

		CustomWeaponShootOpts& secondary = params.shootOpts[1];
		secondary.cooldown = 3135;
		secondary.accuracyX = 2 * 100;
		secondary.accuracyY = 2 * 100;
		secondary.flags = FL_WC_SHOOT_NO_ATTACK | FL_WC_SHOOT_COOLDOWN_IDLE;

		// unsilenced params
		{
			float spread = VECTOR_CONE_2DEGREES.x;
			int damage = 40;

			AddEvent(WepEvt().PrimaryNotEmpty().WepAnim(USP_SHOOT1_UNSIL).AddAnim(USP_SHOOT2_UNSIL).AddAnim(USP_SHOOT3_UNSIL));
			AddEvent(WepEvt().PrimaryEmpty().WepAnim(USP_SHOOTLAST_UNSIL));
			AddEvent(WepEvt().Primary().PlaySound(shootSnd, CHAN_WEAPON, 0.9f, ATTN_NORM, 100, 100, DISTANT_9MM, WC_AIVOL_NORMAL, 0));
			AddEvent(WepEvt().Primary().Bullets(1, 0, damage, spread, spread, 2, WC_FLASH_BRIGHT, 0));

			// using the opposite sound because the server swaps it before it can play
			AddEvent(WepEvt().Secondary().WepAnim(USP_ADD_SILENCER));
			AddEvent(WepEvt().Secondary().Delay(1020).IdleSound(silSnd_alt));

			AddEvent(WepEvt().Reload().Delay(460).IdleSound(reloadSnd1));
			AddEvent(WepEvt().Reload().Delay(1080).IdleSound(reloadSnd2));
			AddEvent(WepEvt().Reload().Delay(2220).IdleSound(reloadSnd3));

			AddEvent(WepEvt().BulletFired().PunchRandom(2.0f, 0));
			AddEvent(WepEvt().BulletFired().EjectShell(shell, TE_BOUNCE_SHELL, 26, -8, 9));

			AddEvent(WepEvt().Deploy().IdleSound(deploySnd));
			AddEvent(WepEvt().Deploy().Delay(540).IdleSound(deploySnd2));

			unsilenced_params = params;
			PrecacheEvents();
		}

		// clear events
		params.numEvents = 0;

		// silenced params
		{
			// This is not accurate to the sven version.
			// Accuracy, recoil, and damage stay the same there. Pretty sure this is cooler though.
			float spread = VECTOR_CONE_1DEGREES.x;
			int damage = 30;

			params.deployAnim = USP_DRAW;
			params.reloadStage[0] = { USP_RELOAD, 2730 };
			params.idles[0] = { USP_IDLE, 99, 2000 };

			primary.accuracyX = 1.2f * 100;
			primary.accuracyY = 1.2f * 100;
			secondary.accuracyX = 1.2f * 100;
			secondary.accuracyY = 1.2f * 100;

			AddEvent(WepEvt().PrimaryNotEmpty().WepAnim(USP_SHOOT1)); // 2 and 3 don't return to idle pose
			AddEvent(WepEvt().PrimaryEmpty().WepAnim(USP_SHOOTLAST));
			AddEvent(WepEvt().Primary().PlaySound(shootSnd_alt, CHAN_WEAPON, 0.9f, ATTN_NORM, 100, 100, DISTANT_9MM, WC_AIVOL_SILENT, 0));
			AddEvent(WepEvt().Primary().Bullets(1, 0, damage, spread, spread, 2, WC_FLASH_NONE, 0));

			AddEvent(WepEvt().Secondary().WepAnim(USP_DETACH_SILENCER));
			AddEvent(WepEvt().Secondary().Delay(780).IdleSound(silSnd));

			AddEvent(WepEvt().Reload().Delay(460).IdleSound(reloadSnd1));
			AddEvent(WepEvt().Reload().Delay(1080).IdleSound(reloadSnd2));
			AddEvent(WepEvt().Reload().Delay(2220).IdleSound(reloadSnd3_alt));

			AddEvent(WepEvt().BulletFired().PunchAdd(-0.8f, 0));
			AddEvent(WepEvt().BulletFired().EjectShell(shell, TE_BOUNCE_SHELL, 26, -8, 9));

			AddEvent(WepEvt().Deploy().IdleSound(deploySnd));
			AddEvent(WepEvt().Deploy().Delay(540).IdleSound(deploySnd2_alt));

			silenced_params = params;
			PrecacheEvents();
		}

		// default without silencer
		params = unsilenced_params;

		UTIL_PrecacheOther("ammo_9mmclip");
	}

	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
		ammoEntName = "ammo_9mmclip";
		dropAmount = USP_MAX_CLIP;
	}

	void SecondaryAttackCustom() {
		CBasePlayer* m_pPlayer = GetPlayer();

		m_silencer = !m_silencer;
		params = m_silencer ? silenced_params : unsilenced_params;

		SendPredictionData(m_pPlayer->edict(), WC_PRED_SEND_BOTH);
		m_pPlayer->SetAnimation(PLAYER_RELOAD, 3.0f);
	}
};


LINK_ENTITY_TO_CLASS(weapon_usp, CUsp);