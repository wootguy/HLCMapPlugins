#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"
#include "weapon_custom.h"
#include "WeaponSound.h"
#include "CWeaponCustomSound.h"
#include "CWeaponCustomShoot.h"
#include "CWeaponCustomEffect.h"
#include "CWeaponCustomUserEffect.h"
#include "CWeaponCustomConfig.h"

void CWeaponCustomShoot::KeyValue(KeyValueData* pkvd)
{
	if (HandleKv(pkvd, "sounds")) parseSounds(pkvd, sounds);
	else if (HandleKv(pkvd, "shoot_fail_snds")) parseSounds(pkvd, shoot_fail_snds);
	else if (HandleKv(pkvd, "shoot_anims"))   parseStrings(pkvd, shoot_anims);
	else if (HandleKv(pkvd, "shoot_empty_snd"))  shoot_empty_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "shoot_empty_anim")) shoot_empty_anim = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "ammo_cost"))     ammo_cost = atoi(pkvd->szValue);
	//else if (HandleKv(pkvd, "shoot_type"))    shoot_type = atoi(pkvd->szValue);			
	else if (HandleKv(pkvd, "cooldown"))      cooldown = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "cooldown_fail")) cooldown_fail = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "recoil"))        UTIL_StringToVector(recoil, pkvd->szValue);
	else if (HandleKv(pkvd, "kickback"))      UTIL_StringToVector(kickback, pkvd->szValue);
	else if (HandleKv(pkvd, "knockback"))     UTIL_StringToVector(knockback, pkvd->szValue);
	else if (HandleKv(pkvd, "max_range"))     max_range = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "heal_mode"))     heal_mode = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "heal_targets"))  heal_targets = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "damage_amt"))  damage = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "damage_type"))  damage_type = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "damage_type2"))  damage_type2 = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "gib_type"))  gib_type = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "friendly_fire"))  friendly_fire = atoi(pkvd->szValue) == 1;

	else if (HandleKv(pkvd, "shell_type")) { shell_type = atoi(pkvd->szValue); update_shell_type(); }
	else if (HandleKv(pkvd, "shell_model"))  shell_model = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "shell_offset")) UTIL_StringToVector(shell_offset, pkvd->szValue);
	else if (HandleKv(pkvd, "shell_vel"))    UTIL_StringToVector(shell_vel, pkvd->szValue);
	else if (HandleKv(pkvd, "shell_spread")) shell_spread = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "shell_delay")) shell_delay = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "shell_delay_snd")) shell_delay_snd.file = ALLOC_STRING(pkvd->szValue);

	else if (HandleKv(pkvd, "bullets"))       bullets = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "bullet_type"))   bullet_type = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "bullet_spread")) bullet_spread = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "bullet_delay"))  bullet_delay = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "bullet_color"))  bullet_color = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "bullet_spread_func"))  bullet_spread_func = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "bullet_decal"))  bullet_decal = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "melee_anims"))   parseStrings(pkvd, melee_anims);
	else if (HandleKv(pkvd, "melee_hit_sounds"))   parseSounds(pkvd, melee_hit_sounds);
	else if (HandleKv(pkvd, "melee_flesh_sounds")) parseSounds(pkvd, melee_flesh_sounds);
	else if (HandleKv(pkvd, "melee_miss_cooldown")) melee_miss_cooldown = atof(pkvd->szValue);

	if (HandleKv(pkvd, "hook_type")) hook_type = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "hook_targets")) hook_targets = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "hook_pull_mode")) hook_pull_mode = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "hook_anim")) hook_anim = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "hook_anim2")) hook_anim2 = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "hook_force")) hook_force = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "hook_speed")) hook_speed = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "hook_max_speed")) hook_max_speed = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "hook_delay")) hook_delay = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "hook_delay2")) hook_delay2 = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "hook_texture_filter")) hook_texture_filter = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "hook_sound")) hook_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "hook_sound2")) hook_snd2.file = ALLOC_STRING(pkvd->szValue);

	else if (HandleKv(pkvd, "projectile_type"))          projectile.type = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_world_event"))   projectile.world_event = (WeaponCustomProjectileAction)atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_monster_event")) projectile.monster_event = (WeaponCustomProjectileAction)atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_speed"))         projectile.speed = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_life"))          projectile.life = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_bounce"))        projectile.elasticity = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_grav"))          projectile.gravity = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_water_friction"))projectile.water_friction = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_air_friction"))  projectile.air_friction = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_class"))         projectile.entity_class = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_mdl"))    		 projectile.model = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_snd"))    		 projectile.move_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_spr"))    		 projectile.sprite = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_spr_color"))     projectile.sprite_color = UTIL_ParseRGBA(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_spr_scale"))     projectile.sprite_scale = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_size"))          projectile.size = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_trail_spr"))     projectile.trail_spr = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_trail_life"))    projectile.trail_life = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_trail_width"))   projectile.trail_width = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_trail_color"))   projectile.trail_color = UTIL_ParseRGBA(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_angles"))  	     UTIL_StringToVector(projectile.angles, pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_avel"))  		 UTIL_StringToVector(projectile.avel, pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_offset"))  	     UTIL_StringToVector(projectile.offset, pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_player_vel_inf")) UTIL_StringToVector(projectile.player_vel_inf, pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_follow_mode"))   projectile.follow_mode = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_follow_radius")) projectile.follow_radius = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_follow_angle"))  projectile.follow_angle = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_follow_time"))   UTIL_StringToVector(projectile.follow_time, pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_trail_effect_freq"))   projectile.trail_effect_freq = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "projectile_dir"))           UTIL_StringToVector(projectile.dir, pkvd->szValue);
	else if (HandleKv(pkvd, "bounce_effect_delay"))      projectile.bounce_effect_delay = atof(pkvd->szValue);

	else if (HandleKv(pkvd, "beam_impact_speed"))       beam_impact_speed = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "beam_impact_spr"))         beam_impact_spr = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "beam_impact_spr_scale"))   beam_impact_spr_scale = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "beam_impact_spr_fps"))     beam_impact_spr_fps = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam_impact_spr_color"))   beam_impact_spr_color = UTIL_ParseRGBA(pkvd->szValue);
	else if (HandleKv(pkvd, "beam_ricochet_limit"))     beam_ricochet_limit = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam_ammo_cooldown"))      beam_ammo_cooldown = atof(pkvd->szValue);

	if (HandleKv(pkvd, "beam1_type"))       beams[0].type = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam1_time"))       beams[0].time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "beam1_spr"))        beams[0].sprite = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "beam1_color"))      beams[0].color = UTIL_ParseRGBA(pkvd->szValue);
	else if (HandleKv(pkvd, "beam1_width"))      beams[0].width = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam1_noise"))      beams[0].noise = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam1_scroll"))     beams[0].scrollRate = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam1_alt_color"))  beams[0].alt_color = UTIL_ParseRGBA(pkvd->szValue);
	else if (HandleKv(pkvd, "beam1_alt_width"))  beams[0].alt_width = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam1_alt_noise"))  beams[0].alt_noise = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam1_alt_scroll")) beams[0].alt_scrollRate = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam1_alt_mode"))   beams[0].alt_mode = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam1_alt_time"))   beams[0].alt_time = atof(pkvd->szValue);

	else if (HandleKv(pkvd, "beam2_type"))       beams[1].type = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam2_time"))       beams[1].time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "beam2_spr"))        beams[1].sprite = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "beam2_color"))      beams[1].color = UTIL_ParseRGBA(pkvd->szValue);
	else if (HandleKv(pkvd, "beam2_width"))      beams[1].width = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam2_noise"))      beams[1].noise = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam2_scroll"))     beams[1].scrollRate = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam2_alt_color"))  beams[1].alt_color = UTIL_ParseRGBA(pkvd->szValue);
	else if (HandleKv(pkvd, "beam2_alt_width"))  beams[1].alt_width = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam2_alt_noise"))  beams[1].alt_noise = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam2_alt_scroll")) beams[1].alt_scrollRate = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam2_alt_mode"))   beams[1].alt_mode = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam2_alt_time"))   beams[1].alt_time = atof(pkvd->szValue);

	else if (HandleKv(pkvd, "effect1_name")) 	  effect1_name = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "effect2_name")) 	  effect2_name = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "effect3_name")) 	  effect3_name = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "effect4_name")) 	  effect4_name = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "user_effect1")) 	  user_effect1_str = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "user_effect2")) 	  user_effect2_str = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "user_effect3")) 	  user_effect3_str = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "user_effect4")) 	  user_effect4_str = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "user_effect5")) 	  user_effect5_str = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "user_effect6")) 	  user_effect6_str = ALLOC_STRING(pkvd->szValue);

	else if (HandleKv(pkvd, "rico_angle")) 	  rico_angle = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "muzzle_flash_color")) UTIL_StringToVector(muzzle_flash_color, pkvd->szValue);
	else if (HandleKv(pkvd, "muzzle_flash_adv"))   UTIL_StringToVector(muzzle_flash_adv, pkvd->szValue);
	else if (HandleKv(pkvd, "toggle_cooldown"))   toggle_cooldown = atof(pkvd->szValue);

	if (HandleKv(pkvd, "windup_time")) 	    windup_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_min_time")) 	windup_min_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "wind_down_time")) 	wind_down_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "wind_down_cancel_time")) wind_down_cancel_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_mult")) 	    windup_mult = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_kick_mult")) 	windup_kick_mult = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_snd")) 	    windup_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_loop_snd")) 	windup_loop_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "wind_down_snd")) 	    wind_down_snd.file = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_pitch_start")) windup_pitch_start = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_pitch_end")) 	windup_pitch_end = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_easing")) 		windup_easing = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_action")) 		windup_action = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_cost")) 		windup_cost = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_anim")) 		windup_anim = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "wind_down_anim")) 	wind_down_anim = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_anim_time")) 	windup_anim_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_anim_loop")) 	windup_anim_loop = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_overcharge_time")) windup_overcharge_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_overcharge_cooldown")) windup_overcharge_cooldown = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_overcharge_action")) windup_overcharge_action = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_overcharge_anim")) windup_overcharge_anim = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_movespeed")) windup_movespeed = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "windup_shoot_movespeed")) windup_shoot_movespeed = atof(pkvd->szValue);

	else CBaseEntity::KeyValue(pkvd);

	// dead code due in the original scripts, but would allow live updates
	/*
	if (szKey == "sounds" or szKey == "melee_hit_sounds" or szKey == "melee_flesh_sounds" or
		szKey == "shoot_fail_snds" or szKey == "windup_snd" or szKey == "wind_down_snd" or
		szKey == "windup_loop_snd" or szKey == "hook_sound" or szKey == "hook_sound2" or
		szKey == "projectile_snd" or szKey == "shell_delay_snd" or szKey == "shoot_empty_snd")
	{
		if (g_map_activated)
			loadExternalSoundSettings();
	}

	if (szKey == "effect1_name" or szKey == "effect2_name" or szKey == "effect3_name" or
		szKey == "effect4_name" or szKey == "user_effect1" or szKey == "user_effect2" or
		szKey == "user_effect3" or szKey == "user_effect4" or szKey == "user_effect5" or
		szKey == "user_effect6")
	{
		if (g_map_activated)
			loadExternalEffectSettings();
	}
	*/
}

bool CWeaponCustomShoot::isPrimary()
{
	CWeaponCustomConfig* config = (CWeaponCustomConfig*)weapon.GetEntity();
	return config && config->fire_settings[0].GetEntity() == this;
}

bool CWeaponCustomShoot::isSecondary()
{
	CWeaponCustomConfig* config = (CWeaponCustomConfig*)weapon.GetEntity();
	return config && config->fire_settings[1].GetEntity() == this;
}

bool CWeaponCustomShoot::isTertiary()
{
	CWeaponCustomConfig* config = (CWeaponCustomConfig*)weapon.GetEntity();
	return config && config->fire_settings[2].GetEntity() == this;
}

void CWeaponCustomShoot::loadExternalSoundSettings()
{
	loadSoundSettings(sounds);
	loadSoundSettings(melee_hit_sounds);
	loadSoundSettings(melee_flesh_sounds);
	loadSoundSettings(shoot_fail_snds);
	loadSoundSettings(windup_snd);
	loadSoundSettings(wind_down_snd);
	loadSoundSettings(windup_loop_snd);
	loadSoundSettings(hook_snd);
	loadSoundSettings(hook_snd2);
	loadSoundSettings(projectile.move_snd);
	loadSoundSettings(shell_delay_snd);
	loadSoundSettings(shoot_empty_snd);
}

void CWeaponCustomShoot::loadExternalEffectSettings()
{
	effect1 = loadEffectSettings((CWeaponCustomEffect*)effect1.GetEntity(), effect1_name);
	effect2 = loadEffectSettings((CWeaponCustomEffect*)effect2.GetEntity(), effect2_name);
	effect3 = loadEffectSettings((CWeaponCustomEffect*)effect3.GetEntity(), effect3_name);
	effect4 = loadEffectSettings((CWeaponCustomEffect*)effect4.GetEntity(), effect4_name);

	user_effect1 = loadUserEffectSettings((CWeaponCustomUserEffect*)user_effect1.GetEntity(), user_effect1_str);
	user_effect2 = loadUserEffectSettings((CWeaponCustomUserEffect*)user_effect2.GetEntity(), user_effect2_str);
	user_effect3 = loadUserEffectSettings((CWeaponCustomUserEffect*)user_effect3.GetEntity(), user_effect3_str);
	user_effect4 = loadUserEffectSettings((CWeaponCustomUserEffect*)user_effect4.GetEntity(), user_effect4_str);
	user_effect5 = loadUserEffectSettings((CWeaponCustomUserEffect*)user_effect5.GetEntity(), user_effect5_str);
	user_effect6 = loadUserEffectSettings((CWeaponCustomUserEffect*)user_effect6.GetEntity(), user_effect6_str);
}

int CWeaponCustomShoot::damageType(int defaultType)
{
	int dtype = defaultType;
	if (damage_type >= 0)
		dtype = damage_type;
	return dtype | damage_type2 | gib_type;
}

WeaponSound* CWeaponCustomShoot::getRandomShootSound()
{
	if (sounds.size == 0)
		return NULL;
	int randIdx = RANDOM_LONG(0, sounds.size - 1);
	return &sounds.data[randIdx];
}

WeaponSound* CWeaponCustomShoot::getRandomMeleeHitSound()
{
	static WeaponSound dummy; // TODO: ???
	if (melee_hit_sounds.size == 0)
		return &dummy;
	int randIdx = RANDOM_LONG(0, melee_hit_sounds.size - 1);
	return &melee_hit_sounds.data[randIdx];
}

WeaponSound* CWeaponCustomShoot::getRandomMeleeFleshSound()
{
	if (melee_flesh_sounds.size == 0)
		return NULL;
	int randIdx = RANDOM_LONG(0, melee_flesh_sounds.size - 1);
	return &melee_flesh_sounds.data[randIdx];
}

WeaponSound* CWeaponCustomShoot::getRandomShootFailSound()
{
	if (shoot_fail_snds.size == 0)
		return NULL;
	int randIdx = RANDOM_LONG(0, shoot_fail_snds.size - 1);
	return &shoot_fail_snds.data[randIdx];
}

void CWeaponCustomShoot::Spawn()
{
	if (g_mapinit_finished && !g_map_activated) {
		UTIL_Remove(this);
		return; // already spawned in MapInit, don't spawn again
	}

	if (!pev->targetname) {
		EALERT(at_warning, "has no targetname and will not be used.");
		return;
	}
	else if (custom_weapon_shoots.get(STRING(pev->targetname))) {
		EALERT(at_warning, "more than weapon_custom_shoot has the targetname '%s'", STRING(pev->targetname));
	}

	// projectiles count as 2 bullets because they can spawn lots of special effects
	int iBullets = shoot_type == SHOOT_BULLETS ? bullets : 0;
	//int iProjectiles = pev.spawnflags & FL_SHOOT_PROJECTILE != 0 ? 2 : 0; 
	float bps = (1.0f / V_max(0.01f, cooldown)) * iBullets;

	if ((int)bps > REC_BULLETS_PER_SECOND) {
		EALERT(at_warning, "bullets per second (%d"
			") is greater than the max recommended (%d)\n"
			"Your game might freeze occasionally with 'Overflow 2048 temporary ents!' spammed in console\n",
			(int)bps, REC_BULLETS_PER_SECOND);
	}

	int iBeams = shoot_type == SHOOT_BEAM ? beam_ricochet_limit + 1 : 0;
	if (beams[1].type != BEAM_DISABLED)
		iBeams *= 2;

	if (iBeams > REC_BEAMS) {
		EALERT(at_warning, " max beams (%s"
			") is greater than the max recommended (%d)\n"
			"Your game might freeze occasionally with 'Overflow beam entity list!' spammed in console\n",
			iBeams, REC_BEAMS);
	}

	// translate plugin type to mod type
	switch (projectile.type) {
	case PROJECTILE_ARGRENADE:
		projectile.type = WC_PROJECTILE_ARGRENADE;
		break;
	case PROJECTILE_BANANA:
		projectile.type = WC_PROJECTILE_BANANA;
		break;
	case PROJECTILE_BOLT:
		projectile.type = WC_PROJECTILE_BOLT;
		break;
	case PROJECTILE_DISPLACER:
		projectile.type = WC_PROJECTILE_DISPLACER;
		break;
	case PROJECTILE_GRENADE:
		projectile.type = WC_PROJECTILE_GRENADE;
		break;
	case PROJECTILE_HORNET:
		projectile.type = WC_PROJECTILE_HORNET;
		break;
	case PROJECTILE_HVR:
		projectile.type = WC_PROJECTILE_HVR;
		break;
	case PROJECTILE_RPG:
		projectile.type = WC_PROJECTILE_RPG;
		break;
	case PROJECTILE_SHOCK:
		projectile.type = WC_PROJECTILE_SHOCK;
		break;
	case PROJECTILE_WEAPON:
		projectile.type = WC_PROJECTILE_WEAPON;
		break;
	case PROJECTILE_TRIPMINE:
		projectile.type = WC_PROJECTILE_TRIPMINE;
		break;
	case PROJECTILE_CUSTOM:
		projectile.type = WC_PROJECTILE_CUSTOM;
		break;
	case PROJECTILE_OTHER:
		projectile.type = WC_PROJECTILE_OTHER;
		break;
	}	

	custom_weapon_shoots.put(STRING(pev->targetname), EHANDLE(edict()));

	Precache();
}

int CWeaponCustomShoot::PrecacheModel(string_t model)
{
	if (model) {
		EALERT(at_aiconsole, "Precaching model: %s\n", STRING(model));
		return PRECACHE_MODEL(STRING(model));
	}
	return -1;
}

void CWeaponCustomShoot::PrecacheSound(string_t sound)
{
	if (sound && strstr(STRING(sound), ".")) {
		EALERT(at_aiconsole, "Precaching sound: %s\n", STRING(sound));
		PRECACHE_SOUND(STRING(sound));
	}
}

void CWeaponCustomShoot::update_shell_type()
{
	switch (shell_type)
	{
	case SHELL_SMALL:
		shell_idx = PRECACHE_MODEL("models/shell.mdl");
		break;
	case SHELL_LARGE:
		shell_idx = PRECACHE_MODEL("models/saw_shell.mdl");
		break;
	case SHELL_SHOTGUN:
		shell_idx = PRECACHE_MODEL("models/shotgunshell.mdl");
		break;
	}
	if (shell_model)
		shell_idx = PRECACHE_MODEL(STRING(shell_model));
}

void CWeaponCustomShoot::Precache()
{
	for (int i = 0; i < sounds.size; i++)
		PrecacheSound(sounds.data[i].file);
	for (int i = 0; i < melee_hit_sounds.size; i++)
		PrecacheSound(melee_hit_sounds.data[i].file);
	for (int i = 0; i < melee_flesh_sounds.size; i++)
		PrecacheSound(melee_flesh_sounds.data[i].file);
	for (int i = 0; i < shoot_fail_snds.size; i++)
		PrecacheSound(shoot_fail_snds.data[i].file);

	PrecacheSound(windup_snd.file);
	PrecacheSound(wind_down_snd.file);
	PrecacheSound(windup_loop_snd.file);
	PrecacheSound(hook_snd.file);
	PrecacheSound(hook_snd2.file);
	PrecacheSound(shell_delay_snd.file);
	PrecacheSound(shoot_empty_snd.file);

	PrecacheSound(projectile.move_snd.file);
	PrecacheModel(beam_impact_spr);
	PrecacheModel(beams[0].sprite);
	PrecacheModel(beams[1].sprite);
	PrecacheModel(shell_model);

	// TODO: PrecacheOther for custom entities

	if (projectile.type == PROJECTILE_ARGRENADE)
		PrecacheModel(ALLOC_STRING("models/grenade.mdl"));
	if (projectile.type == PROJECTILE_MORTAR)
	{
		PrecacheModel(ALLOC_STRING("models/mortarshell.mdl"));
		PrecacheSound(ALLOC_STRING("weapons/ofmortar.wav"));
	}
	if (projectile.type == PROJECTILE_HVR)
		PrecacheModel(ALLOC_STRING("models/HVR.mdl"));

	PrecacheModel(projectile.model);
	PrecacheModel(projectile.sprite);
	if (projectile.trail_spr)
		projectile.trail_sprId = PrecacheModel(projectile.trail_spr);

	if (projectile.entity_class)
		UTIL_PrecacheOther(STRING(projectile.entity_class));

	/* kingpin ball
	PrecacheModel( "sprites/nhth1.spr" );
	PrecacheModel( "sprites/shockwave.spr" );
	PrecacheModel( "sprites/muz7.spr" );
	PrecacheSound( "kingpin/kingpin_seeker_amb.wav" );
	PrecacheSound( "tor/tor-staff-discharge.wav" );
	PrecacheSound( "debris/beamstart14.wav" );
	*/
}

void CWeaponCustomBullet::Spawn()
{
	CWeaponCustomShoot::Spawn();
	shoot_type = SHOOT_BULLETS;
}

void CWeaponCustomMelee::Spawn()
{
	CWeaponCustomShoot::Spawn();
	shoot_type = SHOOT_MELEE;
}

void CWeaponCustomProjectile::Spawn()
{
	CWeaponCustomShoot::Spawn();
	shoot_type = SHOOT_PROJECTILE;
}

void CWeaponCustomBeam::Spawn()
{
	CWeaponCustomShoot::Spawn();
	shoot_type = SHOOT_BEAM;
}

LINK_ENTITY_TO_CLASS(weapon_custom_shoot, CWeaponCustomShoot)
LINK_ENTITY_TO_CLASS(weapon_custom_bullet, CWeaponCustomBullet)
LINK_ENTITY_TO_CLASS(weapon_custom_melee, CWeaponCustomMelee)
LINK_ENTITY_TO_CLASS(weapon_custom_projectile, CWeaponCustomProjectile)
LINK_ENTITY_TO_CLASS(weapon_custom_beam, CWeaponCustomBeam)