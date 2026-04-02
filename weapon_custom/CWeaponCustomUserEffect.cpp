#include "extdll.h"
#include "util.h"
#include "CBasePlayerWeapon.h"
#include "weapon_custom.h"
#include "CWeaponCustomSound.h"
#include "Scheduler.h"
#include "CWeaponCustomUserEffect.h"

void player_revert_glow(EHANDLE h_plr, Vector oldGlow, float oldGlowAmt, bool useOldGlow)
{
	if (!h_plr)
		return;

	CBaseEntity* ent = h_plr;
	if (!ent || !ent->IsPlayer())
		return;

	if (useOldGlow)
	{
		ent->pev->renderamt = oldGlowAmt;
		ent->pev->rendercolor = oldGlow;
	}
	else
		ent->pev->renderfx = 0;
}

void custom_user_effect(EHANDLE h_plr, EHANDLE h_wep, CWeaponCustomUserEffect* effect, bool delayFinished)
{
	if (!effect || !h_plr)
		return;

	CBaseEntity* plrEnt = h_plr;
	CBaseEntity* wepEnt = h_wep;
	CBasePlayer* plr = plrEnt->MyPlayerPointer();
	CBasePlayerWeapon* wep = wepEnt->GetWeaponPtr();
	//WeaponCustomBase* c_wep = cast<WeaponCustomBase@>(CastToScriptClass(wepEnt));

	if (effect->delay > 0 && !delayFinished)
	{
		g_Scheduler.SetTimeout(custom_user_effect, effect->delay, h_plr, h_wep, effect, true);
		return;
	}

	if ((effect->pev->spawnflags & FL_UEFFECT_KILL_ACTIVE) != 0)
	{
		// TODO: Kill active effects
	}

	WeaponSound* snd = effect->getRandomSound();
	if (snd)
	{
		bool userOnly = (effect->pev->spawnflags & FL_UEFFECT_USER_SOUNDS) != 0;
		snd->play(plrEnt, CHAN_STATIC, 1, -1, 0, userOnly);
	}

	// damage 'em
	if (effect->self_damage != 0)
	{
		Vector oldVel = plrEnt->pev->velocity;
		plrEnt->TakeDamage(plrEnt->pev, plrEnt->pev, effect->self_damage, effect->damageType());

		// Idk why this even happens. No matter what the damage type is you get launched into the air
		plrEnt->pev->velocity = oldVel;
	}

	// punch 'em
	if (plrEnt->IsPlayer())
		plrEnt->pev->punchangle = effect->punch_angle;

	// push 'em
	UTIL_MakeVectors(plrEnt->pev->v_angle);
	Vector push = effect->push_vel;
	plrEnt->pev->velocity = plrEnt->pev->velocity + gpGlobals->v_right * push.x + Vector(0, 0, 1) * push.y + gpGlobals->v_forward * push.z;

	// rotate 'em
	if (effect->add_angle != Vector(0, 0, 0) || effect->add_angle_rand != Vector(0, 0, 0) && plrEnt->IsPlayer())
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
	if (effect->action_sprite > 0 && plrEnt->IsPlayer()) {
		//plr->ShowOverheadSprite(effect->action_sprite, effect->action_sprite_height, effect->action_sprite_time);
		ALERT(at_error, "WeaponCustomUserEffect: ShowOverheadSprite not implemented\n");
	}

	// firstperson anim
	if (h_wep && wep && plrEnt->IsAlive() && plrEnt->IsPlayer() && wep->pev->owner)
	{
		ALERT(at_error, "WeaponCustomUserEffect: first-person weapon effects not implemented\n");

		if (effect->v_model || effect->p_model || effect->w_model || effect->w_model_body >= 0)
		{
			/*
			c_wep->v_model_override = effect->v_model;
			c_wep->p_model_override = effect->p_model;
			c_wep->w_model_override = effect->w_model;
			if (effect->w_model_body >= 0)
				c_wep->w_model_body_override = effect->w_model_body;
			c_wep->Deploy(true);
			*/
		}
		//if (effect->wep_anim != -1)
		//	wep->SendWeaponAnim(effect->wep_anim, 0, c_wep->w_body());

		//c_wep->TogglePrimaryFire(effect->primary_mode);
	}

	if (effect->hud_text && plrEnt->IsPlayer())
	{
		UTIL_ClientPrint(plr, print_center, STRING(effect->hud_text));
	}

	// thirdperson anim
	if (effect->anim != -1 && plrEnt->IsPlayer())
	{
		plr->m_Activity = ACT_RELOAD;
		plr->pev->sequence = effect->anim;
		plr->pev->frame = effect->anim_frame;
		plr->ResetSequenceInfo();
		plr->pev->framerate = effect->anim_speed;
	}

	if (effect->fade_mode != -1 && plrEnt->IsPlayer())
	{
		UTIL_ScreenFade(plr, effect->fade_color.ToVector(), effect->fade_time,
			effect->fade_hold, effect->fade_color.a, effect->fade_mode);
	}

	//if (effect->pev->spawnflags & 
	//plr->EnableControl(false);
	//plr->pev->flags ^= 4096;

	if (effect->player_sprite_count > 0 && effect->player_sprite > 0 && plrEnt->IsPlayer())
	{
		int numIntervals = int(effect->player_sprite_time / effect->player_sprite_freq);

		ALERT(at_error, "WeaponCustomUserEffect: player_sprites_effect not implemented\n");
		//player_sprites_effect(h_plr, effect->player_sprite, effect->player_sprite_count);
		//g_Scheduler.SetInterval(player_sprites_effect, effect->player_sprite_freq, numIntervals,
		//	h_plr, effect->player_sprite, effect->player_sprite_count);
	}

	if (effect->glow_time > 0)
	{
		// Remember old glow setting in case user is normally glowing due to map logic or server script
		Vector oldGlow = plr->pev->rendercolor;
		float oldGlowAmt = plr->pev->renderamt;
		bool isGlowing = plr->pev->renderfx == 19;

		plr->pev->renderfx = 19;
		plr->pev->renderamt = effect->glow_amt;
		plr->pev->rendercolor = effect->glow_color;

		g_Scheduler.SetTimeout(player_revert_glow, effect->glow_time, h_plr, oldGlow, oldGlowAmt, isGlowing);
	}

	if (effect->beam_mode != UBEAM_DISABLED && wep)
	{
		ALERT(at_error, "WeaponCustomUserEffect: CreateUserBeam not implemented\n");
		//CreateUserBeam(c_wep->state, effect);
	}

	string_t targetStr = effect->pev->target;
	if (targetStr)
	{
		FireTargets(STRING(targetStr), plrEnt, effect, USE_TYPE(effect->triggerstate));
	}

	if (effect->next_effect)
	{
		custom_user_effect(h_plr, h_wep, (CWeaponCustomUserEffect*)effect->next_effect.GetEntity(), false);
	}
}

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

		*effect = *ef;

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

	if (pActivator && pActivator->IsPlayer() || pCaller && pCaller->IsPlayer())
		custom_user_effect(h_plr, h_wep, this);
}

LINK_ENTITY_TO_CLASS(weapon_custom_user_effect, CWeaponCustomUserEffect)
