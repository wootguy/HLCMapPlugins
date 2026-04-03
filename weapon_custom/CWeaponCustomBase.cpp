#include "extdll.h"
#include "util.h"
#include "weapon_custom.h"
#include "CWeaponCustom.h"
#include "CWeaponCustomConfig.h"
#include "CWeaponCustomSound.h"
#include "CWeaponCustomShoot.h"
#include "te_effects.h"
#include "skill.h"

class CWeaponCustomBase : public CWeaponCustom {
public:
	int infoIdx; // index into global item info
	EHANDLE h_settings;

	void Spawn() override {
		h_settings = *custom_weapons.get(STRING(pev->classname));
		CWeaponCustom::Spawn();
	}

	CWeaponCustomConfig* getSettings() {
		return (CWeaponCustomConfig*)h_settings.GetEntity();
	}

	int GetItemInfo(ItemInfo* info) {
		if (!infoIdx) {
			int* idx = g_wep_name_info_idx.get(STRING(pev->classname));
			if (idx) {
				infoIdx = *idx;
			}
			else {
				EALERT(at_error, "Removing unregistered weapon.\n");
				UTIL_Remove(this);
			}
		}

		*info = g_wep_info[infoIdx];
		return true;
	}

	virtual const char* GetDeathNoticeWeapon() { return "weapon_9mmhandgun"; }

	const char* DisplayName() { return "Test"; }

	WepEvt MakeAnimEvt(WepEvt evt, PodArray<string_t, MAX_KV_ARRAY> anims) {
		evt = evt.WepAnim(atoi(STRING(anims.data[0])), 0, FL_WC_ANIM_ORDERED);

		for (int i = 1; i < anims.size; i++) {
			evt = evt.AddAnim(atoi(STRING(anims.data[i])));
		}

		return evt;
	}

	WepEvt MakeSoundEvt(WepEvt evt, WeaponSound& sound, bool isLoud) {
		SoundOpts opts = sound.getOpts();
		int sndIdx = SOUND_INDEX(STRING(opts.file));

		if (opts.isDefault()) {
			return evt.Delay(opts.delay*1000).IdleSound(sndIdx);
		}
		else {
			return evt.Delay(opts.delay*1000)
				.PlaySound(sndIdx, opts.channel, opts.volume, opts.attn,
					opts.pitch - opts.pitchRand, opts.pitch + opts.pitchRand,
					isLoud ? DISTANT_9MM : DISTANT_NONE, isLoud ? WC_AIVOL_NORMAL : WC_AIVOL_QUIET);

			if (opts.hasNext) {
				EALERT(at_error, "Reload sound chains not implemented\n");
			}
		}
	}

	// 0 = primary, 1 = seconday, 2 = tertiary, 3 = alt primary
	void ConfigureAttack(CWeaponCustomConfig* settings, int attackIdx) {
		CWeaponCustomShoot* config = settings->get_shoot_settings(attackIdx);
		CustomWeaponShootOpts& opts = params.shootOpts[attackIdx];

		if (!config)
			return;

		params.flags |= FL_WC_WEP_HAS_PRIMARY << attackIdx; // delicate but smol		

		int flags = config->pev->spawnflags;
		if (flags & (FL_SHOOT_IF_NOT_DAMAGE | FL_SHOOT_IF_NOT_MISS | FL_SHOOT_NO_MELEE_SOUND_OVERLAP
			| FL_SHOOT_RESPONSIVE_WINDUP | FL_SHOOT_QUAKE_MUZZLEFLASH | FL_SHOOT_PROJ_NO_ORIENT
			| FL_SHOOT_NO_BUBBLES | FL_SHOOT_DETONATE_SATCHELS)) {
			EALERT(at_error, "Unimplemented shoot flags used\n");
		}

		if (config->windup_time) {
			EALERT(at_error, "windups not implemented\n");
		}

		// translate flags
		if (config->can_fire_underwater())			opts.flags |= FL_WC_SHOOT_UNDERWATER;
		if (!(flags & FL_SHOOT_PARTIAL_AMMO_SHOOT)) opts.flags |= FL_WC_SHOOT_NEED_FULL_COST;
		if (flags & FL_SHOOT_NO_AUTOFIRE)			opts.flags |= FL_WC_SHOOT_NO_AUTOFIRE;

		opts.ammoCost = config->ammo_cost;
		opts.ammoFreq = 0;
		opts.ammoPool = 0;
		opts.cooldown = config->cooldown * 1000;
		opts.cooldownFail = config->cooldown_fail * 1000;
		opts.chargeTime = 0;
		opts.chargeCancelTime = 0;
		opts.chargeMoveSpeedMult = 0;
		opts.accuracyX = config->bullet_spread * 100;
		opts.accuracyY = config->bullet_spread * 100;

		WepEvt attackEvt = WepEvt();
		WepEvt attackEmptyEvt = WepEvt();
		WepEvt attackNotEmptyEvt = WepEvt();

		switch (attackIdx) {
		default:
		case 0:
			attackEvt = attackEvt.Primary();
			break;
		case 1:
			attackEvt = attackEvt.Secondary();
			break;
		case 2:
			attackEvt = attackEvt.Tertiary();
			break;
		case 3:
			attackEvt = attackEvt.PrimaryAlt();
			break;
		}

		// animations
		if (config->shoot_empty_anim != -1) {
			if (attackIdx == 0) {
				AddEvent(MakeAnimEvt(WepEvt().PrimaryNotEmpty(), config->shoot_anims));
				AddEvent(WepEvt().PrimaryEmpty().WepAnim(config->shoot_empty_anim));
			}
			else {
				EALERT(at_error, "Only primary attacks can have empty anims\n");
			}
		}
		else {
			AddEvent(MakeAnimEvt(attackEvt, config->shoot_anims));
		}

		// sounds
		if (config->shoot_empty_snd.file) {
			if (attackIdx == 0) {
				AddEvent(MakeSoundEvt(WepEvt().PrimaryEmpty(), config->shoot_empty_snd, true));

				for (int i = 0; i < config->sounds.size; i++) {
					AddEvent(MakeSoundEvt(WepEvt().PrimaryNotEmpty(), config->sounds.data[i], true));
				}
			}
			else {
				EALERT(at_error, "Only primary attacks can have empty sounds\n");
			}
		}
		else {
			for (int i = 0; i < config->sounds.size; i++) {
				AddEvent(MakeSoundEvt(attackEvt, config->sounds.data[i], true));
			}
		}

		// attack
		float spread = UTIL_ConeFromDegrees(config->bullet_spread).x;

		switch (config->shoot_type) {
		case SHOOT_BULLETS:
			AddEvent(attackEvt.Bullets(config->bullets, config->bullet_delay*1000, config->damage,
				spread, spread, 1, WC_FLASH_NORMAL, 0));
			break;
		case SHOOT_MELEE:
			EALERT(at_error, "Melee attacks not implemented\n");
			break;
		case SHOOT_PROJECTILE:
			EALERT(at_error, "Projectile attacks not implemented\n");
			break;
		case SHOOT_BEAM:
			EALERT(at_error, "Beam attacks not implemented\n");
			break;
		}

		// after effects
		float punchRange = fabs(config->recoil[0] - config->recoil[1]);
		float punchMidPoint = config->recoil[0] + (config->recoil[1] - config->recoil[0]) * 0.5f;
		AddEvent(attackEvt.PunchRandom(punchRange * 0.5f, 0));
		AddEvent(attackEvt.PunchAdd(-punchMidPoint, 0));

		if (config->kickback != g_vecZero) {
			//AddEvent(WepEvt().Kickback());
		}
		
		if (config->shell_type != SHELL_NONE) {
			int ishell = MODEL_INDEX(STRING(config->shell_model));
			Vector shellOfs = config->shell_offset;
			AddEvent(attackEvt.Delay(config->shell_delay)
				.EjectShell(ishell, shellOfs.z, shellOfs.y, shellOfs.x));

			if (config->shell_delay > 0 && config->shell_delay_snd.file) {
				SoundOpts opts = config->shell_delay_snd.getOpts();
				WepEvt evt = MakeSoundEvt(attackEvt, config->shell_delay_snd, false);
				AddEvent(evt.Delay((config->shell_delay + opts.delay)*1000));
			}
		}
	}

	void ConfigureWeapon(CWeaponCustomConfig* settings) {
		animExt = settings->getPlayerAnimExt();
		//wrongClientWeapon = "weapon_9mmhandgun";
		EALERT(at_warning, "Wrong client weapon not implemented\n");

		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = settings->deploy_anim;
		params.deployTime = settings->deploy_time * 1000;
		params.maxClip = settings->clip_size();

		if (settings->reload_mode == RELOAD_SIMPLE) {
			params.reloadStage[0] = { (uint8_t)settings->reload_anim, (uint16_t)(settings->reload_time*1000) };

			// delay the next idle if the reload finishes before the animation
			float animDur = GetSequenceDuration(GET_MODEL_PTR(params.vmodel), settings->reload_anim);
			if (animDur > settings->reload_time) {
				AddEvent(WepEvt().Reload().Cooldown(animDur * 1000, FL_WC_COOLDOWN_IDLE));
			}

			WeaponSound& reloadSnd = settings->reload_snd;

			if (reloadSnd.file) {
				AddEvent(MakeSoundEvt(WepEvt().Reload(), reloadSnd, false));
			}
			if (settings->reload_empty_anim >= 0) {
				params.reloadStage[1] = { (uint8_t)settings->reload_empty_anim,  (uint16_t)(settings->reload_time * 1000) };
			}
		}
		else {
			EALERT(at_error, "Special reloads not implemented\n");
		}

		if (settings->deploy_snd.file)
			AddEvent(MakeSoundEvt(WepEvt().Deploy(), settings->deploy_snd, false));

		int idleCount = V_min(4, settings->idle_anims.size);
		if (settings->idle_anims.size > 4) {
			EALERT(at_error, "Too many idle animations\n");
		}

		uint8_t idleChance = 100 / idleCount; // evenly distribute idle chances
		uint16_t idleTime = settings->idle_time * 1000;

		for (int i = 0; i < idleCount; i++) {
			uint8_t anim = atoi(STRING(settings->idle_anims.data[i]));
			params.idles[i] = { anim, idleChance, idleTime };
		}
	}

	void Precache() override {
		CWeaponCustomConfig* settings = getSettings();
		if (!settings)
			return;

		m_defaultModelV = STRING(settings->wpn_v_model);
		m_defaultModelP = STRING(settings->wpn_p_model);
		m_defaultModelW = STRING(settings->wpn_w_model);
		CBasePlayerWeapon::Precache();

		ConfigureWeapon(settings);
		ConfigureAttack(settings, 0);

		PrecacheEvents();
	}

	void GetAmmoDropInfo(bool secondary, const char*& ammoEntName, int& dropAmount) {
		CWeaponCustomConfig* settings = getSettings();
		if (!settings)
			return;

		if (secondary) {
			ammoEntName = STRING(settings->secondary_ammo_drop_class);
			dropAmount = settings->secondary_ammo_drop_amt;
		}
		else {
			ammoEntName = STRING(settings->primary_ammo_drop_class);
			dropAmount = settings->primary_ammo_drop_amt;
		}
	}
};

LINK_ENTITY_TO_CLASS(weapon_custom_base, CWeaponCustomBase)