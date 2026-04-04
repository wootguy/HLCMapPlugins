#include "extdll.h"
#include "util.h"
#include "CBasePlayerWeapon.h"
#include "weapon_custom.h"
#include "CWeaponCustomSound.h"
#include "Scheduler.h"
#include "CWeaponCustomUserEffect.h"

CWeaponCustomUserEffect* loadUserEffectSettings(CWeaponCustomUserEffect* effect, string_t name)
{
	if (effect && effect->valid)
		return effect;

	CBaseEntity* ent = UTIL_FindEntityByTargetname(NULL, STRING(name));
	if (ent)
	{
		CWeaponCustomUserEffect* ef = (CWeaponCustomUserEffect*)ent;
		ef->loadExternalSoundSettings();
		ef->valid = true;

		ef->loadExternalUserEffectSettings(); // load chained effects
		return ef;
	}
	else if (name)
	{
		ALERT(at_error, "WEAPON_CUSTOM ERROR: Failed to find weapon_custom_user_effect '%s'", STRING(name));
	}
	return effect;
}

void CWeaponCustomUserEffect::KeyValue(KeyValueData* pkvd)
{
	if (HandleKv(pkvd, "delay"))       delay = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "sounds"))      parseSounds(pkvd, sounds);

	else if (HandleKv(pkvd, "self_damage"))  self_damage = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "damage_type"))  damage_type = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "damage_type2")) damage_type2 = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "gib_type"))     gib_type = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "primary_mode")) primary_mode = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "hud_text"))  hud_text = ALLOC_STRING(pkvd->szValue);

	else if (HandleKv(pkvd, "beam_mode"))   beam_mode = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam_type"))   beam_type = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam_width"))  beam_width = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam_noise"))  beam_noise = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam_scroll")) beam_scroll = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "beam_time"))   beam_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "beam_start"))  beam_start = UTIL_ParseVector(pkvd->szValue);
	else if (HandleKv(pkvd, "beam_end"))    beam_end = UTIL_ParseVector(pkvd->szValue);
	else if (HandleKv(pkvd, "beam_color"))  beam_color = UTIL_ParseRGBA(pkvd->szValue);
	else if (HandleKv(pkvd, "beam_spr"))    beam_spr = ALLOC_STRING(pkvd->szValue);

	else if (HandleKv(pkvd, "add_angle"))      add_angle = UTIL_ParseVector(pkvd->szValue);
	else if (HandleKv(pkvd, "add_angle_rand")) add_angle_rand = UTIL_ParseVector(pkvd->szValue);
	else if (HandleKv(pkvd, "add_angle_time")) add_angle_time = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "punch_angle"))    punch_angle = UTIL_ParseVector(pkvd->szValue);
	else if (HandleKv(pkvd, "push_vel"))       push_vel = UTIL_ParseVector(pkvd->szValue);

	else if (HandleKv(pkvd, "action_sprite"))        action_sprite = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "action_sprite_height")) action_sprite_height = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "action_sprite_time"))   action_sprite_time = atof(pkvd->szValue);

	else if (HandleKv(pkvd, "fade_mode"))  fade_mode = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "fade_color")) fade_color = UTIL_ParseRGBA(pkvd->szValue);
	else if (HandleKv(pkvd, "fade_hold"))  fade_hold = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "fade_time"))  fade_time = atof(pkvd->szValue);

	else if (HandleKv(pkvd, "wep_anim"))   wep_anim = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "anim"))       anim = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "anim_speed")) anim_speed = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "anim_frame")) anim_frame = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "player_sprite_count")) player_sprite_count = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "player_sprite"))       player_sprite = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "player_sprite_freq"))  player_sprite_freq = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "player_sprite_time"))  player_sprite_time = atof(pkvd->szValue);

	else if (HandleKv(pkvd, "glow_color")) glow_color = UTIL_ParseVector(pkvd->szValue);
	else if (HandleKv(pkvd, "glow_amt"))   glow_amt = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "glow_time"))  glow_time = atof(pkvd->szValue);

	else if (HandleKv(pkvd, "v_model"))  v_model = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "p_model"))  p_model = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "w_model"))  w_model = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "w_model_body"))  w_model_body = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "triggerstate")) triggerstate = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "next_effect")) next_effect_str = ALLOC_STRING(pkvd->szValue);

	else CBaseEntity::KeyValue(pkvd);
}

void CWeaponCustomUserEffect::Spawn()
{
	if (g_mapinit_finished && !g_map_activated) {
		UTIL_Remove(this);
		return; // already spawned in MapInit, don't spawn again
	}

	Precache();
}

void CWeaponCustomUserEffect::loadExternalSoundSettings()
{
	loadSoundSettings(sounds);
}

void CWeaponCustomUserEffect::loadExternalUserEffectSettings()
{
	if (next_effect_loaded)
		return; // fix recursion crash
	next_effect_loaded = true;
	next_effect = loadUserEffectSettings((CWeaponCustomUserEffect*)next_effect.GetEntity(), next_effect_str);
}

WeaponSound* CWeaponCustomUserEffect::getRandomSound()
{
	if (sounds.size == 0)
		return NULL;
	int randIdx = RANDOM_LONG(0, sounds.size - 1);
	return &sounds.data[randIdx];
}

void CWeaponCustomUserEffect::PrecacheSound(string_t sound)
{
	if (sound) {
		EALERT(at_aiconsole, "Precaching sound: %s\n", STRING(sound));
		PRECACHE_SOUND(STRING(sound));
	}
}

int CWeaponCustomUserEffect::PrecacheModel(string_t model)
{
	if (model) {
		EALERT(at_aiconsole, "Precaching model: %s\n", STRING(model));
		return PRECACHE_MODEL(STRING(model));
	}
	return -1;
}

int CWeaponCustomUserEffect::damageType()
{
	return damage_type | damage_type2 | gib_type;
}

void CWeaponCustomUserEffect::Precache()
{
	for (int i = 0; i < sounds.size; i++)
		PrecacheSound(sounds.data[i].file);

	PrecacheModel(action_sprite);
	PrecacheModel(player_sprite);
	PrecacheModel(v_model);
	PrecacheModel(p_model);
	PrecacheModel(w_model);
}

void CWeaponCustomUserEffect::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	EHANDLE h_plr;
	EHANDLE h_wep = NULL;

	if (pCaller && pCaller->IsPlayer())
		h_plr = pCaller;
	else
		h_plr = pActivator;

	EALERT(at_error, "Custom user effect triggering not implemented\n");
	//if (pActivator && pActivator->IsPlayer() || pCaller && pCaller->IsPlayer())
	//	custom_user_effect(h_plr, h_wep, this);
}

LINK_ENTITY_TO_CLASS(weapon_custom_user_effect, CWeaponCustomUserEffect)
