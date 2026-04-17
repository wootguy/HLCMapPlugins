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
		return wrongClientWeapon ? wrongClientWeapon : "weapon_9mmhandgun";
	}

	const char* DisplayName() override { return displayName ? STRING(displayName) : STRING(pev->classname); }

	WepEvt MakeAnimEvt(WepEvt evt, PodArray<string_t, MAX_KV_ARRAY> anims) {
		evt = evt.WepAnim(atoi(STRING(anims.data[0])), 0, FL_WC_ANIM_ORDERED);

		for (int i = 1; i < anims.size; i++) {
			evt = evt.AddAnim(atoi(STRING(anims.data[i])));
		}

		return evt;
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
				AddEvent(evt.IdleSound(sndIdx));
			}
			else {
				AddEvent(evt.PlaySound(sndIdx, opts.channel, opts.volume, opts.attn,
					opts.pitch - opts.pitchRand, opts.pitch + opts.pitchRand,
					isLoud ? DISTANT_9MM : DISTANT_NONE, isLoud ? WC_AIVOL_NORMAL : WC_AIVOL_QUIET, 0));
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
			WepEvt custom = baseEvent.CustomServerLogic(EVT_SELF_DAMAGE);
			custom.server.fuser1 = effect->self_damage;
			custom.server.iuser1 = effect->damageType();
			AddEvent(custom);
		}

		// punch angle
		Vector punch = effect->punch_angle;
		if (punch != g_vecZero) {
			AddEvent(baseEvent.PunchAdd(punch.x, punch.y, punch.z));
		}

		// kickback
		Vector push = effect->push_vel;
		if (push != g_vecZero) {
			Vector norm = push.Normalize();
			AddEvent(baseEvent.Kickback(push.Length(), norm.z * -100, norm.x * 100, 0, norm.y * 100));
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
			AddEvent(baseEvent.WepAnim(effect->wep_anim));
		}

		// primary fire mode toggle
		switch (effect->primary_mode) {
		default:
		case PRIMARY_NO_CHANGE:
			break;
		case PRIMARY_FIRE:
			AddEvent(baseEvent.DisableState(FL_WC_STATE_PRIMARY_ALT));
			break;
		case PRIMARY_ALT_FIRE:
			AddEvent(baseEvent.EnableState(FL_WC_STATE_PRIMARY_ALT));
			break;
		case PRIMARY_TOGGLE:
			AddEvent(baseEvent.ToggleState(FL_WC_STATE_PRIMARY_ALT));
			break;
		}

		// model swap
		if (effect->v_model || effect->p_model || effect->w_model) {
			WepEvt custom = baseEvent.CustomServerLogic(EVT_CHANGE_MODELS);
			custom.server.suser1 = effect->v_model;
			custom.server.suser2 = effect->p_model;
			custom.server.suser3 = effect->w_model;
			AddEvent(custom);
		}
		
		// world model body swap
		if (effect->w_model_body >= 0) {
			WepEvt custom = baseEvent.CustomServerLogic(EVT_CHANGE_W_BODY);
			custom.server.iuser1 = effect->w_model_body;
			AddEvent(custom);
		}

		// hud text
		if (effect->hud_text) {
			WepEvt custom = baseEvent.CustomServerLogic(EVT_CENTER_PRINT);
			custom.server.suser1 = effect->hud_text;
			AddEvent(custom);
		}

		// thirdperson anim
		if (effect->anim != -1) {
			WepEvt custom = baseEvent.CustomServerLogic(EVT_PLAYER_ANIM);
			custom.server.iuser1 = effect->anim;
			custom.server.fuser1 = effect->anim_frame;
			custom.server.fuser2 = effect->anim_speed;
			AddEvent(custom);
		}

		// screen fade
		if (effect->fade_mode != -1) {
			WepEvt custom = baseEvent.CustomServerLogic(EVT_SCREEN_FADE);
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
			WepEvt custom = baseEvent.CustomServerLogic(EVT_GLOW_SHELL);
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
			WepEvt custom = baseEvent.CustomServerLogic(EVT_TRIGGER);
			custom.server.suser1 = targetStr;
			custom.server.iuser1 = effect->triggerstate;
			custom.server.euser1 = EHANDLE(effect->edict());
			AddEvent(custom);
		}

		return AddEffectChainEvents(baseEvent, effect->next_effect, totalDelay, isLoud);
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
		int attackFlag = 0;
		switch (attackIdx) {
		default:
		case 0:
			attackEvt = attackEvt.Primary();
			attackStartEvt = attackStartEvt.PrimaryStart();
			attackEndEvt = attackEndEvt.PrimaryStop();
			attackFailEvt = attackFailEvt.PrimaryFail();
			attackChargeEvt = attackChargeEvt.PrimaryCharge();
			attackOverchargeEvt = attackOverchargeEvt.PrimaryOvercharge();
			attackFlag = FL_WC_WEP_HAS_PRIMARY;
			impactAnyArg = WC_TRIG_IMPACT_PRIMARY_ANY;
			if (settings->primary_empty_snd.file)
				opts.emptySound = SOUND_INDEX(STRING(settings->primary_empty_snd.file));
			break;
		case 1:
			attackEvt = attackEvt.Secondary();
			attackStartEvt = attackStartEvt.SecondaryStart();
			attackEndEvt = attackEndEvt.SecondaryStop();
			attackFailEvt = attackFailEvt.SecondaryFail();
			attackChargeEvt = attackChargeEvt.SecondaryCharge();
			attackOverchargeEvt = attackOverchargeEvt.SecondaryOvercharge();
			attackFlag = FL_WC_WEP_HAS_SECONDARY;
			impactAnyArg = WC_TRIG_IMPACT_SECONDARY_ANY;
			if (settings->secondary_empty_snd.file)
				opts.emptySound = SOUND_INDEX(STRING(settings->secondary_empty_snd.file));
			break;
		case 2:
			attackEvt = attackEvt.Tertiary();
			attackStartEvt = attackStartEvt.Tertiary();
			attackEndEvt = attackEndEvt.Tertiary();
			attackFailEvt = attackFailEvt.Tertiary();
			attackChargeEvt = attackChargeEvt.Tertiary();
			attackOverchargeEvt = attackOverchargeEvt.Tertiary();
			attackFlag = FL_WC_WEP_HAS_TERTIARY;
			impactAnyArg = WC_TRIG_IMPACT_TERTIARY_ANY;
			if (settings->tertiary_empty_snd.file)
				opts.emptySound = SOUND_INDEX(STRING(settings->tertiary_empty_snd.file));
			break;
		case 3:
			attackEvt = attackEvt.PrimaryAlt();
			attackStartEvt = attackStartEvt.PrimaryAlt();
			attackEndEvt = attackEndEvt.PrimaryAlt();
			attackFailEvt = attackFailEvt.PrimaryAlt();
			attackChargeEvt = attackChargeEvt.PrimaryAlt();
			attackOverchargeEvt = attackOverchargeEvt.PrimaryAlt();
			attackFlag = FL_WC_WEP_HAS_ALT_PRIMARY;
			impactAnyArg = WC_TRIG_IMPACT_PRIMARY_ALT_ANY;
			opts.emptySound = params.shootOpts[0].emptySound;
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
			case FIRE_ACT_LASER:
				params.flags |= FL_WC_WEP_UNLINK_COOLDOWNS;
				AddEvent(attackEvt.ToggleState(FL_WC_STATE_LASER | FL_WC_STATE_PRIMARY_ALT));
				return;
			case FIRE_ACT_ZOOM:
				params.flags |= FL_WC_WEP_UNLINK_COOLDOWNS;
				AddEvent(attackEvt.ToggleZoom(settings->zoom_fov));
				AddEvent(attackEvt.ToggleState(FL_WC_STATE_PRIMARY_ALT));
				return;
			case FIRE_ACT_ALT:
				AddEvent(attackEvt.ToggleState(FL_WC_STATE_PRIMARY_ALT));
				return;
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
		opts.accuracyX = config->bullet_spread * 100;
		opts.accuracyY = config->bullet_spread * 100;

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
				AddEvent(MakeAnimEvt(WepEvt().PrimaryNotEmpty(), config->shoot_anims));
				AddEvent(WepEvt().PrimaryEmpty().WepAnim(config->shoot_empty_anim));
			}
			else {
				EALERT(at_error, "Only primary clip attacks can have empty anims\n");
			}
		}
		else {
			WepEvt evt = isConstantBeam ? attackStartEvt : attackEvt;
			AddEvent(MakeAnimEvt(evt, config->shoot_anims));
		}

		// sounds
		bool noSoundOverlap = config->pev->spawnflags & FL_SHOOT_NO_MELEE_SOUND_OVERLAP;
		int forceChannel = noSoundOverlap ? CHAN_WEAPON : -1;
		if (config->shoot_empty_snd.file && !isConstantBeam) {
			if (opts.ammoPool == WC_AMMOPOOL_PRIMARY_CLIP) {
				AddSoundChainEvents(WepEvt().PrimaryEmpty(), config->shoot_empty_snd, 0, true, forceChannel);

				for (int i = 0; i < config->sounds.size; i++) {
					AddSoundChainEvents(WepEvt().PrimaryNotEmpty(), config->sounds.data[i], 0, true, forceChannel);
				}
			}
			else {
				EALERT(at_error, "Only primary clip attacks can have empty sounds\n");
			}
		}
		else {
			WepEvt evt = isConstantBeam ? attackStartEvt : attackEvt;
			for (int i = 0; i < config->sounds.size; i++) {
				AddSoundChainEvents(evt, config->sounds.data[i],0,  true, forceChannel);
			}
		}

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

				AddEvent(attackChargeEvt.PlaySound(sndIdx, CHAN_WEAPON, opts.volume, opts.attn,
					config->windup_pitch_start, config->windup_pitch_end,
					DISTANT_NONE, WC_AIVOL_QUIET, FL_WC_SOUND_CHARGE_PITCH));
			}
			if (config->windup_loop_snd.file) {
				AddSoundChainEvents(attackChargeEvt, config->windup_snd, 0, false, forceChannel);
			}

			AddEvent(attackChargeEvt.WepAnim(config->windup_anim));
			AddEvent(attackChargeEvt.Delay(config->windup_anim_time*1000)
				.WepAnim(config->windup_anim_loop));

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
				AddEvent(attackOverchargeEvt.WepAnim(config->windup_overcharge_anim));
			}
			if (config->windup_overcharge_cooldown) {
				AddEvent(attackOverchargeEvt.Cooldown(config->windup_overcharge_cooldown*1000,
					FL_WC_COOLDOWN_PRIMARY | FL_WC_COOLDOWN_SECONDARY | FL_WC_COOLDOWN_TERTIARY));
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
			bool showTracers = config->bullet_color != -1;
			int tracerColor = config->bullet_color != -1 ? config->bullet_color : WC_TRACER_COLOR_DEFAULT;
			AddEvent(attackEvt
				.Bullets(config->bullets, config->bullet_delay * 1000, config->damage*damageScale,
				spread, spread, showTracers ? 1 : 0, WC_FLASH_NORMAL, 0)
				.BulletColor(tracerColor)
			);
			break;
		}
		case SHOOT_MELEE:
			EALERT(at_error, "Melee attacks not implemented\n");
			break;
		case SHOOT_PROJECTILE: {
			float spread = config->bullet_spread;
			ProjectileOptions opt = config->projectile;
			WeaponCustomProjectile ptype = (WeaponCustomProjectile)opt.type;

			WepEvt evt = attackEvt.Projectile(ptype);
			evt.proj.entity_class = ALLOC_STRING("custom_projectile_plugin");
			evt.proj.speed = opt.speed;
			evt.proj.spreadX = spread;
			evt.proj.spreadY = spread;
			(Vector)evt.proj.offset = opt.offset;
			(Vector)evt.proj.dir = opt.dir;
			evt.proj.gravity = opt.gravity;
			evt.proj.elasticity = opt.elasticity;
			evt.proj.world_event = opt.world_event;
			evt.proj.monster_event = opt.monster_event;
			evt.proj.air_friction = opt.air_friction;
			evt.proj.water_friction = opt.water_friction;
			(Vector)evt.proj.avel = opt.avel;
			evt.proj.life = opt.life;
			evt.proj.size = opt.size;
			evt.proj.sprite = opt.sprite;
			evt.proj.damage = config->damage*damageScale;
			evt.proj.damageBits = config->damage_type | config->damage_type2;
			evt.proj.model = opt.model ? MODEL_INDEX(STRING(opt.model)) : 0;
			evt.proj.sprite_color = opt.sprite_color;
			evt.proj.sprite_scale = opt.sprite_scale;
			(Vector)evt.proj.player_vel_inf = opt.player_vel_inf;
			(Vector)evt.proj.angles = opt.angles;
			evt.proj.trail_spr = opt.trail_spr;
			evt.proj.trail_life = opt.trail_life;
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
			float spread = config->bullet_spread;
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

				int constId = attackIdx * 2 + i + 1;
				int id = opt.time == 0 ? constId : 0;
				constantMode |= opt.time == 0;

				WepEvt beamEvt = attackEvt
					.Beam(id, opt.time * 1000, config->max_range)
					.BeamDamage(config->damage * damageScale, spread, spread, config->beam_impact_speed * 1000)
					.BeamStyle(spriteIdx, opt.color, opt.width, opt.noise, opt.scrollRate, 1, flags);

				if (opt.alt_mode > BEAM_ALT_DISABLED) {
					if (opt.alt_mode == BEAM_ALT_RANDOM) {
						beamEvt = beamEvt.BeamStyleAlt(BEAM_ALT_TOGGLE, 10,
							opt.alt_color, opt.alt_width, opt.alt_noise, opt.alt_scrollRate);
					}
					else {
						beamEvt = beamEvt.BeamStyleAlt(opt.alt_mode, opt.alt_time * 1000,
							opt.alt_color, opt.alt_width, opt.alt_noise, opt.alt_scrollRate);
					}
				}

				if (config->beam_impact_spr && !addedImpactSprite) {
					beamEvt = beamEvt.BeamImpactSprite(MODEL_INDEX(STRING(config->beam_impact_spr)),
						config->beam_impact_spr_fps, config->beam_impact_spr_scale, config->beam_impact_spr_color);
					addedImpactSprite = true;
				}

				AddEvent(beamEvt);
			}

			if (config->beam_ricochet_limit > 0) {
				EALERT(at_error, "Beam ricochets not implemented\n");
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
				AddEvent(attackEndEvt.Cooldown(config->cooldown * 1000, 0xff));

				// finish attack anim
				AddEvent(attackEndEvt.WepAnim(config->hook_anim));
			}

			break;
		}
		}

		// after effects
		float punchRange = fabs(config->recoil[0] - config->recoil[1]);
		float punchMidPoint = config->recoil[0] + (config->recoil[1] - config->recoil[0]) * 0.5f;
		if (punchRange > 0) {
			AddEvent(attackEvt.PunchRandom(punchRange * 0.5f, 0));
			AddEvent(attackEvt.PunchAdd(-punchMidPoint, 0));
		}
		else {
			AddEvent(attackEvt.PunchSet(-punchMidPoint, 0));
		}

		if (config->shoot_type != SHOOT_BULLETS && (config->pev->spawnflags & FL_SHOOT_QUAKE_MUZZLEFLASH)) {
			AddEvent(attackEvt.MuzzleFlash(WC_FLASH_NORMAL));
		}

		if (config->kickback != g_vecZero) {
			bool kickbackScaling = false;
			float force = config->kickback.Length();
			Vector dir = config->kickback.Normalize();
			if (config->windup_time > 0) {
				force *= config->windup_kick_mult;
			}
			AddEvent(attackEvt.Kickback(force, dir.z * -100, dir.x * 100, 0, dir.y * 100));
		}
		
		if (config->shell_type != SHELL_NONE) {
			Vector shellOfs = config->shell_offset;
			int sound = config->shell_type == SHELL_SHOTGUN ? TE_BOUNCE_SHOTSHELL : TE_BOUNCE_SHELL;

			AddEvent(attackEvt.Delay(config->shell_delay)
				.EjectShell(config->shell_idx, sound, shellOfs.z, shellOfs.y, shellOfs.x));

			if (config->shell_delay > 0 && config->shell_delay_snd.file) {
				SoundOpts opts = config->shell_delay_snd.getOpts();
				AddSoundChainEvents(attackEvt, config->shell_delay_snd, config->shell_delay, false);
			}
		}

		// impact effects
		if (config->effect1) {
			CWeaponCustomEffect* ef = (CWeaponCustomEffect*)config->effect1.GetEntity();
			WepEvt impactEvt = WepEvt().Impact(impactAnyArg);

			if (ef->rico_part_count > 0 && ef->rico_part_spr) {
				AddEvent(impactEvt.SpriteTrail(MODEL_INDEX(STRING(ef->rico_part_spr)),
					ef->rico_part_count, ef->rico_part_scale, ef->rico_part_speed, ef->rico_part_speed / 2));
			}
			if (ef->rico_decal == -1) {
				ALERT(at_error, "Player decals not implemented\n");
			}
			if (ef->rico_decal >= 0) {
				const char* decal = getDecal(ef->rico_decal);
				int decalIdx = DECAL_INDEX(decal);

				if (decalIdx != -1) {
					AddEvent(impactEvt.Decal(decalIdx, ef->pev->spawnflags & FL_EFFECT_GUNSHOT_RICOCHET));
				}
				else {
					ALERT(at_error, "Unknown decal: %s\n", decal);
				}
			}
		}
	}

	void ConfigureReload(CWeaponCustomConfig* settings) {
		if (settings->reload_mode == RELOAD_SIMPLE) {
			params.reloadStage[0] = { (uint8_t)settings->reload_anim, (uint16_t)(settings->reload_time * 1000) };

			// delay the next idle if the reload finishes before the animation
			float animDur = GetSequenceDuration(GET_MODEL_PTR(params.vmodel), settings->reload_anim);
			if (animDur > settings->reload_time) {
				AddEvent(WepEvt().Reload().Cooldown(animDur * 1000, FL_WC_COOLDOWN_IDLE));
			}

			if (settings->reload_snd.file) {
				AddSoundChainEvents(WepEvt().Reload(), settings->reload_snd, 0, false);
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
				AddSoundChainEvents(WepEvt().Reload(), settings->reload_snd, 0, false);
			}
			if (settings->reload_end_snd.file) {
				AddSoundChainEvents(WepEvt().ReloadFinish(), settings->reload_end_snd, 0, false);
			}
		}
		else if (settings->reload_mode == RELOAD_STAGED_RESPONSIVE) {
			EALERT(at_error, "Responsive staged reloads not implemented\n");
		}
		else if (settings->reload_mode == RELOAD_EFFECT_CHAIN) {
			if (settings->user_effect1 && settings->user_effect2) {
				float time1 = AddEffectChainEvents(WepEvt().ReloadNotEmpty(), settings->user_effect1, 0, false);
				float time2 = AddEffectChainEvents(WepEvt().ReloadEmpty(), settings->user_effect2, 0, false);

				params.reloadStage[0] = { 0, (uint16_t)(time1 * 1000) };
				params.reloadStage[1] = { 0, (uint16_t)(time2 * 1000) };
			}
			else {
				float time = AddEffectChainEvents(WepEvt().Reload(), settings->user_effect1, 0, false);
				params.reloadStage[0] = { 0, (uint16_t)(time * 1000) };
			}
		}
	}

	void ConfigureWeapon(CWeaponCustomConfig* settings) {
		animExt = settings->getPlayerAnimExt();
		wrongClientWeapon = settings->hl_client_weapon ? STRING(settings->hl_client_weapon) : NULL;

		if (!wrongClientWeapon) {
			EALERT(at_warning, "HL Client weapon not set\n");
		}

		params.vmodel = MODEL_INDEX(GetModelV());
		params.deployAnim = settings->deploy_anim;
		params.deployTime = settings->deploy_time * 1000;
		params.maxClip = settings->clip_size();

		ConfigureReload(settings);

		if (settings->deploy_snd.file)
			AddSoundChainEvents(WepEvt().Deploy(), settings->deploy_snd, 0, false);

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

	void AttackTrace(CBasePlayer* plr, int attackIdx, Vector vecSrc, TraceResult& tr) override {
		if (tr.flFraction >= 1.0f)
			return;

		CWeaponCustomConfig* settings = getSettings();
		if (!settings)
			return;

		CWeaponCustomShoot* config = settings->get_shoot_settings(attackIdx);
		if (!config)
			return;

		CBaseEntity* pEntity = CBaseEntity::Instance(tr.pHit);

		EHANDLE effect = pEntity->IsMonster() ? config->effect2 : config->effect1;
		Vector vecDir = (tr.vecEndPos - vecSrc).Normalize();
		custom_effect(tr.vecEndPos, effect, NULL, tr.pHit, plr->edict(), vecDir, config->friendly_fire ? 1 : 0);
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
