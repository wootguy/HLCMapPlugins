#include "extdll.h"
#include "util.h"
#include "weapon_custom.h"
#include "CWeaponCustom.h"
#include "CWeaponCustomConfig.h"
#include "CWeaponCustomSound.h"
#include "CWeaponCustomShoot.h"
#include "CWeaponCustomEffect.h"
#include "CWeaponCustomUserEffect.h"
#include "te_effects.h"
#include "skill.h"
#include "Scheduler.h"

enum CustomServerEvents {
	EVT_SELF_DAMAGE,		// apply damage to user
	EVT_CHANGE_MODELS,		// change weapon models
	EVT_CHANGE_W_BODY,		// change world model body
	EVT_PLAYER_ANIM,		// player model animation
	EVT_CENTER_PRINT,		// center print HUD text
	EVT_SCREEN_FADE,
	EVT_GLOW_SHELL,			// player model glow shell effect
	EVT_TRIGGER,			// trigger entity by name
};

const char* cstr(string_t s) {
	return STRING(s);
}

class CWeaponCustomBase : public CWeaponCustom {
public:
	int infoIdx; // index into global item info
	EHANDLE h_settings;
	string_t displayName;
	bool predictedEffects[4][2]; // maps [attackIdx][effect_num] to true if effect is predicted by the client

	void Spawn() override {
		h_settings = *custom_weapons.get(STRING(pev->classname));

		CWeaponCustomConfig* settings = getSettings();
		if (settings) {
			displayName = settings->display_name;
		}
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

	virtual const char* GetDeathNoticeWeapon() override {
		return params.wrongClientWeapon ? STRING(params.wrongClientWeapon) : "weapon_9mmhandgun";
	}

	const char* DisplayName() override { return displayName ? STRING(displayName) : STRING(pev->classname); }

	WepEvt MakeAnimEvt(WepEvt evt, PodArray<string_t, MAX_KV_ARRAY> anims) {
		WepEvt animEvt = evt.Type(WC_EVT_WEP_ANIM);
		animEvt.anim.anims.arr[0] = atoi(STRING(anims.data[0]));
		animEvt.anim.anims.arrSz = anims.size;
		animEvt.anim.flags = FL_WC_ANIM_ORDERED;

		for (int i = 1; i < anims.size; i++) {
			animEvt.anim.anims.arr[i] = atoi(STRING(anims.data[i]));
		}

		return animEvt;
	}

	void AddSoundChainEvents(WepEvt evt, WeaponSound& sound, float totalDelay, bool isLoud, int forceChannel=-1) {
		SoundOpts opts = sound.getOpts();
		int sndIdx = SOUND_INDEX(STRING(opts.file));

		if (forceChannel != -1)
			opts.channel = forceChannel; // Hacky but this flag is redundant and should be removed anyway

		totalDelay += opts.delay;
		evt = evt.Delay(totalDelay*1000);

		if (opts.file) {
			if (opts.isDefault()) {
				WepEvt idleSoundEvt = evt.Type(WC_EVT_IDLE_SOUND);
				idleSoundEvt.idleSound.sound = sndIdx;
				AddEvent(idleSoundEvt);
			}
			else {
				WepEvt soundEvt = evt.Type(WC_EVT_PLAY_SOUND);
				soundEvt.playSound.sound = sndIdx;
				soundEvt.playSound.channel = opts.channel;
				soundEvt.playSound.aiVol = isLoud ? WC_AIVOL_NORMAL : WC_AIVOL_QUIET;
				soundEvt.playSound.volume = (int)(opts.volume * 255.5f);
				soundEvt.playSound.attn = clampf(opts.attn * 64, 0, 255.0f);
				soundEvt.playSound.pitchMin = opts.pitch - opts.pitchRand;
				soundEvt.playSound.pitchMax = opts.pitch + opts.pitchRand;
				soundEvt.playSound.distantSound = isLoud ? DISTANT_9MM : DISTANT_NONE;
				soundEvt.playSound.flags = 0;
				AddEvent(soundEvt);
			}
		}

		if (opts.nextSnd) {
			CWeaponCustomSound* wsound = (CWeaponCustomSound*)opts.nextSnd.GetEntity();
			WeaponSound next = wsound->getWeaponSound();
			AddSoundChainEvents(evt, next, totalDelay, isLoud, forceChannel);
		}
	}

	float AddEffectChainEvents(WepEvt baseEvent, EHANDLE h_effect, float totalDelay, bool isLoud, int forceChannel=-1) {
		CWeaponCustomUserEffect* effect = (CWeaponCustomUserEffect*)h_effect.GetEntity();
		if (!effect)
			return totalDelay;

		totalDelay += effect->delay;
		baseEvent = baseEvent.Delay(totalDelay * 1000);

		// sounds
		if (effect->sounds.size) {
			bool userOnly = (effect->pev->spawnflags & FL_UEFFECT_USER_SOUNDS) != 0;
			if (userOnly) {
				EALERT(at_error, "User only sounds not implemented\n");
			}

			for (int i = 0; i < effect->sounds.size; i++) {
				// TODO: select randomly from here, don't add them all
				AddSoundChainEvents(baseEvent, effect->sounds.data[i], totalDelay, isLoud, forceChannel);
			}
		}

		// self damage
		if (effect->self_damage != 0) {
			WepEvt custom = baseEvent.Type(WC_EVT_SERVER);
			custom.server.type = EVT_SELF_DAMAGE;
			custom.server.fuser1 = effect->self_damage;
			custom.server.iuser1 = effect->damageType();
			AddEvent(custom);
		}

		// punch angle
		Vector punch = effect->punch_angle;
		if (punch != g_vecZero) {
			WepEvt punchEvt = baseEvent.Type(WC_EVT_PUNCH);
			punchEvt.recoil.flags = FL_WC_PUNCH_ADD;
			punchEvt.recoil.angles[0] = FLOAT_TO_FP_10_6(punch.x);
			punchEvt.recoil.angles[1] = FLOAT_TO_FP_10_6(punch.y);
			punchEvt.recoil.angles[2] = FLOAT_TO_FP_10_6(punch.z);
			AddEvent(punchEvt);
		}

		// kickback
		Vector push = effect->push_vel;
		if (push != g_vecZero) {
			Vector norm = push.Normalize();

			WepEvt kickbackEvt = baseEvent.Type(WC_EVT_KICKBACK);
			kickbackEvt.kickback.pushForce = push.Length();
			kickbackEvt.kickback.back = clamp(norm.z * -100, -100, 100);
			kickbackEvt.kickback.right = clamp(norm.x * 100, -100, 100);
			kickbackEvt.kickback.up = 0;
			kickbackEvt.kickback.globalUp = clamp(norm.y * 100, -100, 100);
			AddEvent(kickbackEvt);
		}

		// view angle animation
		if (effect->add_angle != g_vecZero || effect->add_angle_rand != g_vecZero)
		{
			float startTime = gpGlobals->time;
			float endTime = startTime + effect->add_angle_time;
			Vector r = effect->add_angle_rand;

			Vector randAngle = Vector(RANDOM_FLOAT(-r.x, r.x), RANDOM_FLOAT(-r.y, r.y), RANDOM_FLOAT(-r.z, r.z));
			Vector addAngle = effect->add_angle + randAngle;
			ALERT(at_error, "WeaponCustomUserEffect: animate_view_angles not implemented\n")
				//g_Scheduler.SetTimeout(animate_view_angles, 0, h_plr, plrEnt->pev->v_angle, addAngle, startTime, endTime);
		}

		// indicate something
		if (effect->action_sprite > 0) {
			//plr->ShowOverheadSprite(effect->action_sprite, effect->action_sprite_height, effect->action_sprite_time);
			ALERT(at_error, "WeaponCustomUserEffect: ShowOverheadSprite not implemented\n");
		}

		// firstperson anim
		if (effect->wep_anim != -1) {
			WepEvt animEvt = baseEvent.Type(WC_EVT_WEP_ANIM);
			animEvt.anim.anims.arr[0] = effect->wep_anim;
			animEvt.anim.anims.arrSz = 1;
			AddEvent(animEvt);
		}

		// primary fire mode toggle
		WepEvt primToggleEvt = baseEvent.clone();
		primToggleEvt.evtType = WC_EVT_TOGGLE_STATE;
		primToggleEvt.toggleState.stateBits = FL_WC_STATE_PRIMARY_ALT;

		switch (effect->primary_mode) {
		default:
		case PRIMARY_NO_CHANGE:
			break;
		case PRIMARY_FIRE:
			primToggleEvt.toggleState.toggleMode = WC_TOGGLE_STATE_OFF;
			AddEvent(primToggleEvt);
			break;
		case PRIMARY_ALT_FIRE:
			primToggleEvt.toggleState.toggleMode = WC_TOGGLE_STATE_ON;
			AddEvent(primToggleEvt);
			break;
		case PRIMARY_TOGGLE:
			primToggleEvt.toggleState.toggleMode = WC_TOGGLE_STATE_TOGGLE;
			AddEvent(primToggleEvt);
			break;
		}

		// model swap
		if (effect->v_model || effect->p_model || effect->w_model) {
			WepEvt custom = baseEvent.Type(WC_EVT_SERVER);
			custom.server.type = EVT_CHANGE_MODELS;
			custom.server.suser1 = effect->v_model;
			custom.server.suser2 = effect->p_model;
			custom.server.suser3 = effect->w_model;
			AddEvent(custom);
		}
		
		// world model body swap
		if (effect->w_model_body >= 0) {
			WepEvt custom = baseEvent.Type(WC_EVT_SERVER);
			custom.server.type = EVT_CHANGE_W_BODY;
			custom.server.iuser1 = effect->w_model_body;
			AddEvent(custom);
		}

		// hud text
		if (effect->hud_text) {
			WepEvt custom = baseEvent.Type(WC_EVT_SERVER);
			custom.server.type = EVT_CENTER_PRINT;
			custom.server.suser1 = effect->hud_text;
			AddEvent(custom);
		}

		// thirdperson anim
		if (effect->anim != -1) {
			WepEvt custom = baseEvent.Type(WC_EVT_SERVER);
			custom.server.type = EVT_PLAYER_ANIM;
			custom.server.iuser1 = effect->anim;
			custom.server.fuser1 = effect->anim_frame;
			custom.server.fuser2 = effect->anim_speed;
			AddEvent(custom);
		}

		// screen fade
		if (effect->fade_mode != -1) {
			WepEvt custom = baseEvent.Type(WC_EVT_SERVER);
			custom.server.type = EVT_SCREEN_FADE;
			custom.server.cuser1 = effect->fade_color;
			custom.server.fuser1 = effect->fade_time;
			custom.server.fuser2 = effect->fade_hold;
			custom.server.iuser1 = effect->fade_mode;
			AddEvent(custom);
		}

		if (effect->player_sprite_count > 0 && effect->player_sprite > 0)
		{
			int numIntervals = int(effect->player_sprite_time / effect->player_sprite_freq);

			ALERT(at_error, "WeaponCustomUserEffect: player_sprites_effect not implemented\n");
			//player_sprites_effect(h_plr, effect->player_sprite, effect->player_sprite_count);
			//g_Scheduler.SetInterval(player_sprites_effect, effect->player_sprite_freq, numIntervals,
			//	h_plr, effect->player_sprite, effect->player_sprite_count);
		}

		if (effect->glow_time > 0)
		{
			WepEvt custom = baseEvent.Type(WC_EVT_SERVER);
			custom.server.type = EVT_GLOW_SHELL;
			custom.server.iuser1 = effect->glow_amt;
			(Vector)custom.server.vuser1 = effect->glow_color;
			custom.server.fuser1 = effect->glow_time;
			AddEvent(custom);
		}

		if (effect->beam_mode != UBEAM_DISABLED)
		{
			ALERT(at_error, "WeaponCustomUserEffect: CreateUserBeam not implemented\n");
			//CreateUserBeam(c_wep->state, effect);
		}

		string_t targetStr = effect->pev->target;
		if (targetStr) {
			WepEvt custom = baseEvent.Type(WC_EVT_SERVER);
			custom.server.type = EVT_TRIGGER;
			custom.server.suser1 = targetStr;
			custom.server.iuser1 = effect->triggerstate;
			custom.server.euser1 = EHANDLE(effect->edict());
			AddEvent(custom);
		}

		return AddEffectChainEvents(baseEvent, effect->next_effect, totalDelay, isLoud);
	}

	bool AddImpactEffect(WepEvt impactEvt, CWeaponCustomEffect* ef) {
		if (!ef)
			return false;

		bool predicted = false;

		if (ef->rico_part_count > 0 && ef->rico_part_spr) {
			WepEvt spriteTrailEvt = impactEvt.Type(WC_EVT_SPRITETRAIL);
			spriteTrailEvt.spriteTrail.sprite = MODEL_INDEX(STRING(ef->rico_part_spr));
			spriteTrailEvt.spriteTrail.count = ef->rico_part_count;
			spriteTrailEvt.spriteTrail.scale = ef->rico_part_scale;
			spriteTrailEvt.spriteTrail.speed = ef->rico_part_speed;
			spriteTrailEvt.spriteTrail.speedNoise = ef->rico_part_speed / 2;
			AddEvent(spriteTrailEvt);

			predicted = true;
		}
		if (ef->rico_decal == -1) {
			ALERT(at_error, "Player decals not implemented\n");
		}
		if (ef->rico_decal >= 0) {
			const char* decal = getDecal(ef->rico_decal);
			int decalIdx = DECAL_INDEX(decal);

			if (decalIdx != -1) {
				WepEvt decalEvt = impactEvt.Type(WC_EVT_DECAL);
				decalEvt.decal.flags = (ef->pev->spawnflags & FL_EFFECT_GUNSHOT_RICOCHET) ? FL_WC_DECAL_PARTICLES : 0;
				decalEvt.decal.decalIdx = decalIdx;
				AddEvent(decalEvt);

				predicted = true;
			}
			else {
				ALERT(at_error, "Unknown decal: %s\n", decal);
			}
		}

		return predicted;
	}

	// 0 = primary, 1 = seconday, 2 = tertiary, 3 = alt primary
	void ConfigureAttack(CWeaponCustomConfig* settings, int attackIdx) {
		CustomWeaponShootOpts& opts = params.shootOpts[attackIdx];
		
		WepEvt attackEvt = WepEvt();
		WepEvt attackStartEvt = WepEvt();
		WepEvt attackEndEvt = WepEvt();
		WepEvt attackFailEvt = WepEvt();
		WepEvt attackChargeEvt = WepEvt();
		WepEvt attackOverchargeEvt = WepEvt();
		WepEvt attackFinishEvt = WepEvt();
		int impactAnyArg = 0;
		int impactMonsterArg = 0;
		int attackFlag = 0;
		switch (attackIdx) {
		default:
		case 0:
			attackEvt = WepEvt(WC_TRIG_PRIMARY);
			attackStartEvt = WepEvt(WC_TRIG_PRIMARY_START);
			attackEndEvt = WepEvt(WC_TRIG_PRIMARY_STOP);
			attackFailEvt = WepEvt(WC_TRIG_PRIMARY_FAIL);
			attackChargeEvt = WepEvt(WC_TRIG_PRIMARY_CHARGE);
			attackOverchargeEvt = WepEvt(WC_TRIG_PRIMARY_OVERCHARGE);
			attackFlag = FL_WC_WEP_HAS_PRIMARY;
			impactAnyArg = WC_TRIG_IMPACT_PRIMARY_ANY;
			impactMonsterArg = WC_TRIG_IMPACT_PRIMARY_MONSTER;
			if (settings->primary_empty_snd.file)
				opts.emptySound = SOUND_INDEX(STRING(settings->primary_empty_snd.file));
			break;
		case 1:
			attackEvt = WepEvt(WC_TRIG_SECONDARY);
			attackStartEvt = WepEvt(WC_TRIG_SECONDARY_START);
			attackEndEvt = WepEvt(WC_TRIG_SECONDARY_STOP);
			attackFailEvt = WepEvt(WC_TRIG_SECONDARY_FAIL);
			attackChargeEvt = WepEvt(WC_TRIG_SECONDARY_CHARGE);
			attackOverchargeEvt = WepEvt(WC_TRIG_SECONDARY_OVERCHARGE);
			attackFlag = FL_WC_WEP_HAS_SECONDARY;
			impactAnyArg = WC_TRIG_IMPACT_SECONDARY_ANY;
			impactMonsterArg = WC_TRIG_IMPACT_SECONDARY_MONSTER;
			if (settings->secondary_empty_snd.file)
				opts.emptySound = SOUND_INDEX(STRING(settings->secondary_empty_snd.file));
			break;
		case 2:
			attackEvt = WepEvt(WC_TRIG_TERTIARY);
			attackStartEvt = WepEvt(WC_TRIG_TERTIARY);
			attackEndEvt = WepEvt(WC_TRIG_TERTIARY);
			attackFailEvt = WepEvt(WC_TRIG_TERTIARY);
			attackChargeEvt = WepEvt(WC_TRIG_TERTIARY);
			attackOverchargeEvt = WepEvt(WC_TRIG_TERTIARY);
			attackFlag = FL_WC_WEP_HAS_TERTIARY;
			impactAnyArg = WC_TRIG_IMPACT_TERTIARY_ANY;
			impactMonsterArg = WC_TRIG_IMPACT_TERTIARY_MONSTER;
			if (settings->tertiary_empty_snd.file)
				opts.emptySound = SOUND_INDEX(STRING(settings->tertiary_empty_snd.file));
			break;
		case 3:
			attackEvt = WepEvt(WC_TRIG_PRIMARY_ALT);
			attackStartEvt = WepEvt(WC_TRIG_PRIMARY_ALT);
			attackEndEvt = WepEvt(WC_TRIG_PRIMARY_ALT);
			attackFailEvt = WepEvt(WC_TRIG_PRIMARY_ALT);
			attackChargeEvt = WepEvt(WC_TRIG_PRIMARY_ALT);
			attackOverchargeEvt = WepEvt(WC_TRIG_PRIMARY_ALT);
			attackFlag = FL_WC_WEP_HAS_ALT_PRIMARY;
			impactAnyArg = WC_TRIG_IMPACT_PRIMARY_ALT_ANY;
			impactMonsterArg = WC_TRIG_IMPACT_PRIMARY_ALT_MONSTER;
			break;
		}

		int action = FIRE_ACT_SHOOT;
		if (attackIdx == 1) {
			action = settings->secondary_action;
		}
		else if (attackIdx == 2) {
			action = settings->tertiary_action;
		}
		
		if (action != FIRE_ACT_SHOOT) {
			CWeaponCustomShoot* alt_config = settings->get_alt_shoot_settings(0);

			params.flags |= attackFlag;
			opts.flags |= FL_WC_SHOOT_NO_ATTACK;
			opts.cooldown = alt_config->toggle_cooldown*1000;

			switch (action) {
			case FIRE_ACT_LASER: {
				params.flags |= FL_WC_WEP_UNLINK_COOLDOWNS;

				WepEvt toggleEvt = attackEvt.clone();
				toggleEvt.evtType = WC_EVT_TOGGLE_STATE;
				toggleEvt.toggleState.toggleMode = WC_TOGGLE_STATE_TOGGLE;
				toggleEvt.toggleState.stateBits = FL_WC_STATE_LASER | FL_WC_STATE_PRIMARY_ALT;
				AddEvent(toggleEvt);
				return;
			}
			case FIRE_ACT_ZOOM: {
				params.flags |= FL_WC_WEP_UNLINK_COOLDOWNS;

				WepEvt toggleEvt = attackEvt.clone();
				toggleEvt.evtType = WC_EVT_TOGGLE_STATE;
				toggleEvt.toggleState.toggleMode = WC_TOGGLE_STATE_TOGGLE;
				toggleEvt.toggleState.stateBits = FL_WC_STATE_PRIMARY_ALT;
				AddEvent(toggleEvt);

				WepEvt zoomEvt = attackEvt.clone();
				zoomEvt.evtType = WC_EVT_TOGGLE_ZOOM;
				zoomEvt.zoomToggle.zoomFov = settings->zoom_fov;
				AddEvent(toggleEvt);
				return;
			}
			case FIRE_ACT_ALT: {
				WepEvt toggleEvt = attackEvt.clone();
				toggleEvt.evtType = WC_EVT_TOGGLE_STATE;
				toggleEvt.toggleState.toggleMode = WC_TOGGLE_STATE_TOGGLE;
				toggleEvt.toggleState.stateBits = FL_WC_STATE_PRIMARY_ALT;
				AddEvent(toggleEvt);
				return;
			}
			case FIRE_ACT_WINDUP:
				if (attackIdx == 2) {
					EALERT(at_error, "Tertiary chargup action not implemented\n");
				}
				params.flags |= FL_WC_WEP_LINK_CHARGEUPS;
				opts.cooldown = params.shootOpts[0].cooldown;
				opts.chargeTime = params.shootOpts[0].chargeTime;
				opts.chargeCancelTime = params.shootOpts[0].chargeCancelTime;
				opts.chargeMoveSpeedMult = params.shootOpts[0].chargeMoveSpeedMult;
				return;
			}
		}

		CWeaponCustomShoot* config = settings->get_shoot_settings(attackIdx);

		if (!config)
			return;

		params.flags |= attackFlag;

		int flags = config->pev->spawnflags;
		if (flags & (FL_SHOOT_IF_NOT_DAMAGE | FL_SHOOT_IF_NOT_MISS | FL_SHOOT_DETONATE_SATCHELS)) {
			EALERT(at_error, "Unimplemented shoot flags used\n");
		}

		if ((flags & FL_SHOOT_NO_BUBBLES) && config->shoot_type != SHOOT_PROJECTILE) {
			EALERT(at_error, "No bubbles flag not implemented for bullets/beams\n");
		}

		// translate flags
		if (config->can_fire_underwater())			opts.flags |= FL_WC_SHOOT_UNDERWATER;
		if (!(flags & FL_SHOOT_PARTIAL_AMMO_SHOOT)) opts.flags |= FL_WC_SHOOT_NEED_FULL_COST;
		if (flags & FL_SHOOT_NO_AUTOFIRE)			opts.flags |= FL_WC_SHOOT_NO_AUTOFIRE;

		// general setup
		opts.ammoCost = config->ammo_cost;
		opts.ammoFreq = 0;
		opts.cooldown = config->cooldown * 1000;
		opts.cooldownFail = config->cooldown_fail * 1000;
		opts.chargeTime = 0;
		opts.chargeCancelTime = 0;
		opts.chargeMoveSpeedMult = 0;
		opts.accuracy[0] = config->bullet_spread * 100;
		opts.accuracy[1] = config->bullet_spread * 100;

		// ammo pool
		if (attackIdx == 0) {
			if (settings->clip_size()) {
				opts.ammoPool = WC_AMMOPOOL_PRIMARY_CLIP;
			}
			else {
				opts.ammoPool = WC_AMMOPOOL_PRIMARY_RESERVE;
			}
		}
		else if (attackIdx == 1) {
			if (settings->clip_size2 == 0) {
				if (!strcmp(STRING(settings->primary_ammo_type), STRING(settings->secondary_ammo_type))) {
					opts.ammoPool = params.shootOpts[0].ammoPool;
				}
				else {
					opts.ammoPool = WC_AMMOPOOL_SECONDARY_RESERVE;
				}					
			}
			else {
				EALERT(at_error, "Secondary clip not implemented\n");
			}
		}
		else if (attackIdx == 2) {
			if (settings->tertiary_ammo_type == TAMMO_SAME_AS_PRIMARY) {
				opts.ammoPool = params.shootOpts[0].ammoPool;
			}
			else if (settings->tertiary_ammo_type == TAMMO_SAME_AS_SECONDARY) {
				opts.ammoPool = params.shootOpts[1].ammoPool;
			}
		}
		else if (attackIdx == 3) {
			opts.ammoPool = params.shootOpts[0].ammoPool;
		}

		bool isConstantBeam = config->isConstantBeamAttack();

		// animations
		if (config->shoot_empty_anim != -1 && !isConstantBeam) {
			if (opts.ammoPool == WC_AMMOPOOL_PRIMARY_CLIP) {
				AddEvent(MakeAnimEvt(WepEvt(WC_TRIG_PRIMARY_CLIP_SP, WC_TRIG_CLIP_ARG_NOT_EMPTY), config->shoot_anims));
				
				WepEvt animEvt = WepEvt(WC_TRIG_PRIMARY_CLIPSIZE, 0).Type(WC_EVT_WEP_ANIM);
				animEvt.anim.anims.arr[0] = config->shoot_empty_anim;
				animEvt.anim.anims.arrSz = 1;
				AddEvent(animEvt);
			}
			else {
				EALERT(at_error, "Only primary clip attacks can have empty anims\n");
			}
		}
		else {
			WepEvt evt = isConstantBeam ? attackStartEvt : attackEvt;
			AddEvent(MakeAnimEvt(evt.clone(), config->shoot_anims));
		}

		// sounds
		bool noSoundOverlap = config->pev->spawnflags & FL_SHOOT_NO_MELEE_SOUND_OVERLAP;
		int forceChannel = noSoundOverlap ? CHAN_WEAPON : -1;
		if (config->shoot_empty_snd.file && !isConstantBeam) {
			if (opts.ammoPool == WC_AMMOPOOL_PRIMARY_CLIP) {
				AddSoundChainEvents(WepEvt(WC_TRIG_PRIMARY_CLIPSIZE, 0), config->shoot_empty_snd, 0, true, forceChannel);

				for (int i = 0; i < config->sounds.size; i++) {
					AddSoundChainEvents(WepEvt(WC_TRIG_PRIMARY_CLIP_SP, WC_TRIG_CLIP_ARG_NOT_EMPTY), config->sounds.data[i], 0, true, forceChannel);
				}
			}
			else {
				EALERT(at_error, "Only primary clip attacks can have empty sounds\n");
			}
		}
		else {
			WepEvt evt = isConstantBeam ? attackStartEvt : attackEvt;
			for (int i = 0; i < config->sounds.size; i++) {
				AddSoundChainEvents(evt, config->sounds.data[i], 0,  true, forceChannel);
			}
		}

		if (attackIdx == 3)
			opts.emptySound = params.shootOpts[0].emptySound;

		float damageScale = 1.0f;

		// windup / chargeup
		if (config->windup_time > 0) {
			opts.chargeTime = config->windup_time * 1000;
			opts.chargeCancelTime = config->windup_min_time * 1000;

			if (!(config->pev->spawnflags & FL_SHOOT_RESPONSIVE_WINDUP)) {
				opts.chargeCancelTime = opts.chargeTime;
			}
			
			int action = config->windup_action;
			if (action == WINDUP_SHOOT_ON_RELEASE)		opts.chargeMode = WC_CHARGEUP_HOLD;
			if (action == WINDUP_SHOOT_ONCE)			opts.chargeMode = WC_CHARGEUP_SINGLE;
			if (action == WINDUP_SHOOT_CONSTANT)		opts.chargeMode = WC_CHARGEUP_CONSTANT;
			if (action == WINDUP_SHOOT_ONCE_IF_HELD)	opts.chargeMode = WC_CHARGEUP_SINGLE_HOLD;

			if (config->windup_movespeed != 1.0f)
				opts.chargeMoveSpeedMult = config->windup_movespeed * 65535;

			if (config->windup_snd.file) {
				SoundOpts opts = config->windup_snd.getOpts();
				int sndIdx = SOUND_INDEX(STRING(opts.file));

				WepEvt soundEvt = attackChargeEvt.clone().Type(WC_EVT_PLAY_SOUND);
				soundEvt.playSound.sound = sndIdx;
				soundEvt.playSound.channel = CHAN_WEAPON;
				soundEvt.playSound.aiVol = WC_AIVOL_QUIET;
				soundEvt.playSound.volume = (int)(opts.volume * 255.5f);
				soundEvt.playSound.attn = clampf(opts.attn * 64, 0, 255.0f);
				soundEvt.playSound.pitchMin = config->windup_pitch_start;
				soundEvt.playSound.pitchMax = config->windup_pitch_end;
				soundEvt.playSound.distantSound = DISTANT_NONE;
				soundEvt.playSound.flags = FL_WC_SOUND_CHARGE_PITCH;
				AddEvent(soundEvt);
			}
			if (config->windup_loop_snd.file) {
				AddSoundChainEvents(attackChargeEvt, config->windup_snd, 0, false, forceChannel);
			}

			WepEvt animEvt = attackChargeEvt.clone().Type(WC_EVT_WEP_ANIM);
			animEvt.anim.anims.arr[0] = config->windup_anim;
			animEvt.anim.anims.arrSz = 1;
			AddEvent(animEvt);

			WepEvt animEvt2 = animEvt.Delay(config->windup_anim_time * 1000);
			animEvt2.anim.anims.arr[0] = config->windup_anim_loop;
			animEvt2.anim.anims.arrSz = 1;
			AddEvent(animEvt2);

			if (config->windup_overcharge_action == OVERCHARGE_SHOOT) {
				ALERT(at_error, "Overcharge shoot action not implemented\n");
			}
			if (config->windup_overcharge_action == OVERCHARGE_CANCEL) {
				opts.overchargeMode = WC_OVERCHARGE_CANCEL;
			}
			if (config->windup_overcharge_action == OVERCHARGE_CONTINUE) {
				opts.overchargeMode = WC_OVERCHARGE_CONTINUE;
			}
			if (config->user_effect2) {
				opts.overchargeTime = config->windup_overcharge_time*1000;
				AddEffectChainEvents(attackOverchargeEvt, config->user_effect2, 0, false);
			}
			if (config->windup_overcharge_anim != -1) {
				WepEvt animEvt = attackOverchargeEvt.clone().Type(WC_EVT_WEP_ANIM);
				animEvt.anim.anims.arr[0] = config->windup_overcharge_anim;
				animEvt.anim.anims.arrSz = 1;
				AddEvent(animEvt);
			}
			if (config->windup_overcharge_cooldown) {
				WepEvt cooldownEvt = attackOverchargeEvt.clone().Type(WC_EVT_COOLDOWN);
				cooldownEvt.cooldown.millis = config->windup_overcharge_cooldown * 1000;
				cooldownEvt.cooldown.targets = FL_WC_COOLDOWN_PRIMARY | FL_WC_COOLDOWN_SECONDARY | FL_WC_COOLDOWN_TERTIARY;
				AddEvent(cooldownEvt);
			}

			if (config->windup_cost) {
				opts.chargeAmmoMode = WC_CHARGE_AMMO_LOAD;
				opts.ammoCost = config->windup_cost;
			}
			else if (config->ammo_cost) {
				opts.chargeAmmoMode = WC_CHARGE_AMMO_ATTACK;
			}

			damageScale = config->windup_mult;
			opts.chargeFlags |= FL_WC_CHARGE_DAMAGE | FL_WC_CHARGE_KICKBACK;
		}

		// attack
		float spread = UTIL_ConeFromDegrees(config->bullet_spread).x;

		switch (config->shoot_type) {
		case SHOOT_BULLETS: {
			WepEvt bulletEvt = attackEvt.clone();
			bulletEvt.evtType = WC_EVT_BULLETS;
			bulletEvt.bullets.count = config->bullets;
			bulletEvt.bullets.burstDelay = config->bullet_delay * 1000;
			bulletEvt.bullets.damage = config->damage * damageScale;
			bulletEvt.bullets.accuracy[0] = FLOAT_TO_SPREAD(spread);
			bulletEvt.bullets.accuracy[1] = FLOAT_TO_SPREAD(spread);
			bulletEvt.bullets.tracerFreq = config->bullet_color != -1 ? 1 : 0;
			bulletEvt.bullets.tracerColor = config->bullet_color != -1 ? config->bullet_color : WC_TRACER_COLOR_DEFAULT;
			bulletEvt.bullets.flashSz = WC_FLASH_NORMAL;
			bulletEvt.bullets.flags = 0;
			AddEvent(bulletEvt);
			break;
		}
		case SHOOT_MELEE:
			EALERT(at_error, "Melee attacks not implemented\n");
			break;
		case SHOOT_PROJECTILE: {
			ProjectileOptions opt = config->projectile;
			WeaponCustomProjectile ptype = (WeaponCustomProjectile)opt.type;

			WepEvt evt = attackEvt.clone().Type(WC_EVT_PROJECTILE);
			evt.proj.type = ptype;
			evt.proj.entity_class = ALLOC_STRING("custom_projectile_plugin");
			evt.proj.speed = opt.speed;
			evt.proj.accuracy[0] = FLOAT_TO_SPREAD(spread);
			evt.proj.accuracy[1] = FLOAT_TO_SPREAD(spread);
			*(Vector*)evt.proj.offset = opt.offset;
			*(Vector*)evt.proj.dir = opt.dir;
			evt.proj.gravity = opt.gravity;
			evt.proj.elasticity = opt.elasticity;
			evt.proj.world_event = opt.world_event;
			evt.proj.monster_event = opt.monster_event;
			evt.proj.air_friction = opt.air_friction;
			evt.proj.water_friction = opt.water_friction;
			*(Vector*)evt.proj.avel = opt.avel;
			evt.proj.life = opt.life * 1000;
			evt.proj.size = opt.size;
			evt.proj.damage = config->damage*damageScale;
			evt.proj.damageBits = config->damage_type | config->damage_type2;
			evt.proj.model = opt.model ? MODEL_INDEX(STRING(opt.model)) : 0;
			if (opt.sprite) {
				evt.proj.sprite = opt.sprite;
				evt.proj.sprite_color = opt.sprite_color;
				evt.proj.sprite_scale = opt.sprite_scale;
			}
			*(Vector*)evt.proj.player_vel_inf = opt.player_vel_inf;
			*(Vector*)evt.proj.angles = opt.angles;
			evt.proj.trail_spr = MODEL_INDEX(STRING(opt.trail_spr));
			evt.proj.trail_life = opt.trail_life*100;
			evt.proj.trail_width = opt.trail_width;
			evt.proj.trail_color = opt.trail_color;
			evt.proj.follow_mode = opt.follow_mode;

			if (config->pev->spawnflags & FL_SHOOT_PROJ_NO_ORIENT) {
				evt.proj.flags |= FL_WC_PROJ_NO_ORIENT;
			}
			if (config->pev->spawnflags & FL_SHOOT_NO_BUBBLES) {
				evt.proj.flags |= FL_WC_PROJ_NO_BUBBLES;
			}
			
			if (ptype == WC_PROJECTILE_OTHER) {
				if (opt.entity_class) {
					evt.proj.entity_class = opt.entity_class;
				}
				else {
					EALERT(at_error, "Projectile class not set. Defaulting to hand grenade.\n");
					evt.proj.type = WC_PROJECTILE_GRENADE;
				}
			}

			AddEvent(evt);
			
			break;
		}
		case SHOOT_BEAM: {
			bool constantMode = false;
			bool addedImpactSprite = false;

			for (int i = 0; i < 2; i++) {
				BeamOptions& opt = config->beams[i];
				int flags = 0;
				uint16_t spriteIdx = MODEL_INDEX(STRING(opt.sprite));

				if (opt.type == BEAM_DISABLED)
					continue;
				if (opt.type == BEAM_SPIRAL || opt.type == BEAM_SPIRAL_OPAQUE)
					flags |= FL_WC_BEAM_SPIRAL;
				if (opt.type == BEAM_LINEAR_OPAQUE || opt.type == BEAM_SPIRAL_OPAQUE)
					flags |= FL_WC_BEAM_OPAQUE;
				if (i > 0)
					flags |= FL_WC_BEAM_NO_EVTS; // decorative beam

				int constId = attackIdx * 2 + i + 1;
				int id = opt.time == 0 ? constId : 0;
				constantMode |= opt.time == 0;

				WepEvt beamEvt = attackEvt.clone();
				beamEvt.evtType = WC_EVT_BEAM;
				beamEvt.beam.id = id;
				beamEvt.beam.life = opt.time * 1000;
				beamEvt.beam.distance = config->max_range;
				beamEvt.beam.sprite = spriteIdx;
				beamEvt.beam.color = opt.color;
				beamEvt.beam.width = opt.width;
				beamEvt.beam.noise = opt.noise;
				beamEvt.beam.scrollRate = opt.scrollRate;
				beamEvt.beam.attachment = 1;
				beamEvt.beam.flags = flags;
				beamEvt.beam.damage = config->damage * damageScale;
				beamEvt.beam.accuracy[0] = FLOAT_TO_SPREAD(spread);
				beamEvt.beam.accuracy[1] = FLOAT_TO_SPREAD(spread);
				beamEvt.beam.freq = config->beam_impact_speed * 1000;

				float ricoSpread = UTIL_ConeFromDegrees(config->rico_angle).x;
				beamEvt.beam.ricoBeams = config->beam_ricochet_limit;
				beamEvt.beam.ricoAngle = FLOAT_TO_SPREAD(ricoSpread);
				beamEvt.beam.hasRicoBeams = config->beam_ricochet_limit > 0;

				if (opt.alt_mode > BEAM_ALT_DISABLED) {
					if (opt.alt_mode == BEAM_ALT_RANDOM) {
						beamEvt.beam.altMode = BEAM_ALT_TOGGLE;
						beamEvt.beam.altTime = 10;
						beamEvt.beam.colorAlt = opt.alt_color;
						beamEvt.beam.widthAlt = opt.alt_width;
						beamEvt.beam.noiseAlt = opt.alt_noise;
						beamEvt.beam.scrollRateAlt = opt.alt_scrollRate;
					}
					else {
						beamEvt.beam.altMode = opt.alt_mode;
						beamEvt.beam.altTime = opt.alt_time * 1000;
						beamEvt.beam.colorAlt = opt.alt_color;
						beamEvt.beam.widthAlt = opt.alt_width;
						beamEvt.beam.noiseAlt = opt.alt_noise;
						beamEvt.beam.scrollRateAlt = opt.alt_scrollRate;
					}
				}

				if (config->beam_impact_spr && !addedImpactSprite && isConstantBeam) {
					beamEvt.beam.hasImpactSprite = 1;
					beamEvt.beam.impactSprite = MODEL_INDEX(STRING(config->beam_impact_spr));
					beamEvt.beam.impactSpriteFps = V_min(127, config->beam_impact_spr_fps);
					beamEvt.beam.impactSpriteScale = config->beam_impact_spr_scale;
					beamEvt.beam.impactSpriteColor = config->beam_impact_spr_color;
					addedImpactSprite = true;
				}

				AddEvent(beamEvt);
			}

			if (config->beam_ricochet_limit > 0 && config->rico_angle > 0 && constantMode) {
				EALERT(at_error, "Beam ricochets not implemented for constant beams\n");
			}

			// running sound
			if (config->hook_snd.file) {
				AddSoundChainEvents(attackStartEvt, config->hook_snd, config->hook_delay, false, forceChannel);
			}

			// end sound
			if (config->hook_snd2.file) {
				AddSoundChainEvents(attackEndEvt, config->hook_snd2, 0, false, forceChannel);
			}

			if (constantMode) {
				// use normal cooldown for spending ammo, and an event to cooldown attacks
				opts.cooldown = config->beam_ammo_cooldown * 1000;

				WepEvt cooldownEvt = attackEndEvt.clone().Type(WC_EVT_COOLDOWN);
				cooldownEvt.cooldown.millis = config->cooldown * 1000;
				cooldownEvt.cooldown.targets = 0xff;
				AddEvent(cooldownEvt);

				// finish attack anim
				WepEvt animEvt = attackEndEvt.clone().Type(WC_EVT_WEP_ANIM);
				animEvt.anim.anims.arr[0] = config->hook_anim;
				animEvt.anim.anims.arrSz = 1;
				AddEvent(animEvt);
			}

			break;
		}
		}

		// after effects
		float punchRange = fabs(config->recoil[0] - config->recoil[1]);
		float punchMidPoint = config->recoil[0] + (config->recoil[1] - config->recoil[0]) * 0.5f;
		if (punchRange > 0) {
			WepEvt punchEvt = attackEvt.clone().Type(WC_EVT_PUNCH);
			punchEvt.recoil.angles[0] = FLOAT_TO_FP_10_6(punchRange * 0.5f);
			AddEvent(punchEvt);

			punchEvt.recoil.flags = FL_WC_PUNCH_ADD;
			punchEvt.recoil.angles[0] = FLOAT_TO_FP_10_6(-punchMidPoint);
			AddEvent(punchEvt);
		}
		else {
			WepEvt punchEvt = attackEvt.clone().Type(WC_EVT_PUNCH);
			punchEvt.recoil.flags = FL_WC_PUNCH_SET;
			punchEvt.recoil.angles[0] = FLOAT_TO_FP_10_6(-punchMidPoint);
			AddEvent(punchEvt);
		}

		if (config->shoot_type != SHOOT_BULLETS && (config->pev->spawnflags & FL_SHOOT_QUAKE_MUZZLEFLASH)) {
			WepEvt muzzleFlashEvt = attackEvt.clone();
			muzzleFlashEvt.evtType = WC_EVT_MUZZLEFLASH;
			muzzleFlashEvt.muzzleFlash.brightness = WC_FLASH_NORMAL;
			AddEvent(muzzleFlashEvt);
		}

		if (config->kickback != g_vecZero) {
			bool kickbackScaling = false;
			float force = config->kickback.Length();
			Vector dir = config->kickback.Normalize();
			if (config->windup_time > 0) {
				force *= config->windup_kick_mult;
			}

			WepEvt kickbackEvt = attackEvt.clone().Type(WC_EVT_KICKBACK);
			kickbackEvt.kickback.pushForce = force;
			kickbackEvt.kickback.back = clamp(dir.z * -100, -100, 100);
			kickbackEvt.kickback.right = clamp(dir.x * 100, -100, 100);
			kickbackEvt.kickback.up = 0;
			kickbackEvt.kickback.globalUp = clamp(dir.y * 100, -100, 100);
			AddEvent(kickbackEvt);
		}
		
		if (config->shell_type != SHELL_NONE) {
			Vector shellOfs = config->shell_offset;
			int sound = config->shell_type == SHELL_SHOTGUN ? TE_BOUNCE_SHOTSHELL : TE_BOUNCE_SHELL;

			WepEvt ejectEvt = attackEvt.clone().Type(WC_EVT_EJECT_SHELL).Delay(config->shell_delay * 1000);
			ejectEvt.ejectShell.model = config->shell_idx;
			ejectEvt.ejectShell.sound = sound;
			ejectEvt.ejectShell.offset[0] = shellOfs.z;
			ejectEvt.ejectShell.offset[1] = shellOfs.y;
			ejectEvt.ejectShell.offset[2] = shellOfs.x;
			AddEvent(ejectEvt);

			if (config->shell_delay > 0 && config->shell_delay_snd.file) {
				SoundOpts opts = config->shell_delay_snd.getOpts();
				AddSoundChainEvents(attackEvt, config->shell_delay_snd, config->shell_delay, false);
			}
		}

		// impact effects
		if (config->effect1) {
			CWeaponCustomEffect* ef = (CWeaponCustomEffect*)config->effect1.GetEntity();			
			if (AddImpactEffect(WepEvt(WC_TRIG_IMPACT, impactAnyArg), ef)) {
				predictedEffects[attackIdx][0] = true;
			}
		}
		if (config->effect2) {
			CWeaponCustomEffect* ef = (CWeaponCustomEffect*)config->effect2.GetEntity();

			if (config->shoot_type == SHOOT_BULLETS) {
				AddImpactEffect(WepEvt(WC_TRIG_IMPACT, impactMonsterArg), ef);
				predictedEffects[attackIdx][1] = true;
			}
			else if (config->shoot_type == SHOOT_BEAM) {
				AddImpactEffect(WepEvt(WC_TRIG_RICOCHET, impactAnyArg), ef);
				predictedEffects[attackIdx][1] = true;
			}
		}
	}

	void ConfigureReload(CWeaponCustomConfig* settings) {
		if (settings->reload_mode == RELOAD_SIMPLE) {
			params.reloadStage[0] = { (uint8_t)settings->reload_anim, (uint16_t)(settings->reload_time * 1000) };

			// delay the next idle if the reload finishes before the animation
			float animDur = GetSequenceDuration(GET_MODEL_PTR(params.vmodel), settings->reload_anim);
			if (animDur > settings->reload_time) {
				WepEvt cooldownEvt = WepEvt(WC_TRIG_RELOAD).Type(WC_EVT_COOLDOWN);
				cooldownEvt.cooldown.millis = animDur * 1000;
				cooldownEvt.cooldown.targets = FL_WC_COOLDOWN_IDLE;

				AddEvent(cooldownEvt);
			}

			if (settings->reload_snd.file) {
				AddSoundChainEvents(WepEvt(WC_TRIG_RELOAD), settings->reload_snd, 0, false);
			}
			if (settings->reload_empty_anim >= 0) {
				params.reloadStage[1] = { (uint8_t)settings->reload_empty_anim,  (uint16_t)(settings->reload_time * 1000) };
			}
		}
		else if (settings->reload_mode == RELOAD_STAGED || settings->reload_mode == RELOAD_STAGED_RESPONSIVE) {
			params.flags |= FL_WC_WEP_SHOTGUN_RELOAD;
			params.reloadStage[0] = { (uint8_t)settings->reload_start_anim, (uint16_t)(settings->reload_start_time*1000) };
			params.reloadStage[1] = { (uint8_t)settings->reload_anim, (uint16_t)(settings->reload_time * 1000) };
			params.reloadStage[2] = { (uint8_t)settings->reload_end_anim, (uint16_t)(settings->reload_end_time * 1000) };

			if (settings->reload_mode == RELOAD_STAGED_RESPONSIVE) {
				ALERT(at_error, "Responsive reloads not implemented\n");
				// TODO: add reload cancel stage
			}

			if (settings->reload_snd.file) {
				AddSoundChainEvents(WepEvt(WC_TRIG_RELOAD), settings->reload_snd, 0, false);
			}
			if (settings->reload_end_snd.file) {
				AddSoundChainEvents(WepEvt(WC_TRIG_RELOAD_FINISH), settings->reload_end_snd, 0, false);
			}
		}
		else if (settings->reload_mode == RELOAD_STAGED_RESPONSIVE) {
			EALERT(at_error, "Responsive staged reloads not implemented\n");
		}
		else if (settings->reload_mode == RELOAD_EFFECT_CHAIN) {
			if (settings->user_effect1 && settings->user_effect2) {
				float time1 = AddEffectChainEvents(WepEvt(WC_TRIG_RELOAD_NOT_EMPTY), settings->user_effect1, 0, false);
				float time2 = AddEffectChainEvents(WepEvt(WC_TRIG_RELOAD_EMPTY), settings->user_effect2, 0, false);

				params.reloadStage[0] = { 0, (uint16_t)(time1 * 1000) };
				params.reloadStage[1] = { 0, (uint16_t)(time2 * 1000) };
			}
			else {
				float time = AddEffectChainEvents(WepEvt(WC_TRIG_RELOAD), settings->user_effect1, 0, false);
				params.reloadStage[0] = { 0, (uint16_t)(time * 1000) };
			}
		}
	}

	void ConfigureWeapon(CWeaponCustomConfig* settings) {
		params.animExt = ALLOC_STRING(settings->getPlayerAnimExt());
		params.wrongClientWeapon = settings->hl_client_weapon ? settings->hl_client_weapon : 0;

		if (!params.wrongClientWeapon) {
			EALERT(at_warning, "HL Client weapon not set\n");
		}

		params.deployAnim = settings->deploy_anim;
		params.deployTime = settings->deploy_time * 1000;
		params.ammoInfo[0].maxClip = settings->clip_size();
		params.ammoInfo[0].defaultGive = settings->default_ammo;

		ConfigureReload(settings);

		if (settings->deploy_snd.file)
			AddSoundChainEvents(WepEvt(WC_TRIG_DEPLOY), settings->deploy_snd, 0, false);

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

		if (settings->pev->spawnflags & FL_WEP_EXCLUSIVE_HOLD) {
			params.flags |= FL_WC_WEP_EXCLUSIVE_HOLD;
		}

		ConfigureAttack(settings, 0);
		ConfigureAttack(settings, 1);
		ConfigureAttack(settings, 2);
		ConfigureAttack(settings, 3);
	}

	void Precache() override {
		CWeaponCustomConfig* settings = getSettings();
		if (!settings)
			return;

		m_defaultModelV = STRING(settings->wpn_v_model);
		m_defaultModelP = STRING(settings->wpn_p_model);
		m_defaultModelW = STRING(settings->wpn_w_model);
		CBasePlayerWeapon::Precache();

		params.vmodel = MODEL_INDEX(GetModelV());

		ConfigureWeapon(settings);

		PrecacheEvents();

		studiohdr_t* hdr = GET_MODEL_PTR(MODEL_INDEX(GetModelV()));
		if (hdr) {
			params.deployAnimTime = GetSequenceDuration(hdr, params.deployAnim) * 1000;
		}
	}

	void CustomServerEvent(WepEvt& evt, CBasePlayer* m_pPlayer) override {
		switch (evt.server.type) {
		case EVT_SELF_DAMAGE:
			m_pPlayer->TakeDamage(pev, pev, evt.server.fuser1, evt.server.iuser1);
			break;
		case EVT_CHANGE_MODELS:
			m_customModelV = evt.server.suser1;
			m_customModelP = evt.server.suser2;
			m_customModelW = evt.server.suser3;
			Deploy();
			break;
		case EVT_CHANGE_W_BODY:
			pev->body = evt.server.iuser1;
			break;
		case EVT_PLAYER_ANIM:
			m_pPlayer->m_Activity = ACT_RELOAD;
			m_pPlayer->pev->sequence = evt.server.iuser1;
			m_pPlayer->pev->frame = evt.server.fuser1;
			m_pPlayer->ResetSequenceInfo();
			m_pPlayer->pev->framerate = evt.server.fuser2;
			break;
		case EVT_CENTER_PRINT:
			UTIL_ClientPrint(m_pPlayer, print_center, STRING(evt.server.suser1));
			break;
		case EVT_SCREEN_FADE:
			UTIL_ScreenFade(m_pPlayer, evt.server.cuser1.ToVector(), evt.server.fuser1,
				evt.server.fuser2, evt.server.cuser1.a, evt.server.iuser1);
			break;
		case EVT_GLOW_SHELL: {
			Vector c = evt.server.vuser1;
			m_pPlayer->AddShockEffect(c.x, c.y, c.z, evt.server.iuser1, evt.server.fuser1);
			break;
		}
		case EVT_TRIGGER:
			FireTargets(STRING(evt.server.suser1), m_pPlayer, evt.server.euser1, (USE_TYPE)evt.server.iuser1);
			break;
		}
	}

	void AttackTrace(CBasePlayer* plr, int attackIdx, Vector vecSrc, TraceResult& tr, bool isRicochet) override {
		if (tr.flFraction >= 1.0f)
			return;

		CWeaponCustomConfig* settings = getSettings();
		if (!settings)
			return;

		CWeaponCustomShoot* config = settings->get_shoot_settings(attackIdx);
		if (!config)
			return;

		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		EHANDLE effect = NULL;
		int effectIdx = 0;

		if (config->shoot_type == SHOOT_BULLETS) {
			effect = pEntity->IsMonster() ? config->effect2 : config->effect1;
			effectIdx = pEntity->IsMonster() ? 1 : 0;
		}
		else if (config->shoot_type == SHOOT_BEAM) {
			effect = isRicochet ? config->effect2 : config->effect1;
			effectIdx = isRicochet ? 1 : 0;
		}

		if (plr->IsSevenKewpClient() && predictedEffects[attackIdx][effectIdx])
			return; // weapon custom will handle this effect

		if (effect) {
			Vector vecDir = (tr.vecEndPos - vecSrc).Normalize();
			custom_effect(tr.vecEndPos, effect, NULL, tr.pHit, plr->edict(), vecDir, config->friendly_fire ? 1 : 0);
		}
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
