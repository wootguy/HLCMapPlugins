#include "extdll.h"
#include "util.h"
#include "CBaseEntity.h"
#include "weapon_custom.h"
#include "CWeaponCustomEffect.h"
#include "CWeaponCustomSound.h"
#include "Scheduler.h"
#include "te_effects.h"
#include "weapons.h"

// custom effect flags
int FL_EFFECT_FRIENDLY_FIRE = 1;
int FL_EFFECT_DELAY_FINISHED = 2;

void custom_explosion(Vector pos, Vector vel, CWeaponCustomEffect* effect, Vector decalPos,
	CBaseEntity* decalEnt, EHANDLE owner, bool inWater, bool friendlyFire)
{
	if (!effect->valid)
		return;

	int smokeScale = int(effect->explode_smoke_spr_scale * 10.0f);
	int smokeFps = int(effect->explode_smoke_spr_fps);
	int expScale = int(effect->explode_spr_scale * 10.0f);
	int expFps = int(effect->explode_spr_fps);
	string_t expSprite = effect->explode_spr;
	if (inWater && effect->explode_water_spr)
		expSprite = effect->explode_water_spr;
	int life = 8;

	if (expSprite)
	{
		switch (effect->explosion_style)
		{
		case EXPLODE_SPRITE_PARTICLES:
		case EXPLODE_SPRITE:
		{
			int flags = 2 | 4; // no sound or lights
			if (effect->explosion_style == EXPLODE_SPRITE)
				flags |= 8; // no particles
			UTIL_Explosion(pos, MODEL_INDEX(STRING(expSprite)), expScale, expFps, flags);
			break;
		}
		case EXPLODE_DISK:
			UTIL_BeamDisk(pos, effect->explode_beam_radius, MODEL_INDEX(STRING(expSprite)),
				effect->explode_beam_frame, effect->explode_beam_fps,
				effect->explode_beam_life, effect->explode_beam_width,
				effect->explode_beam_noise, effect->explode_beam_color,
				effect->explode_beam_scroll);
			break;
		case EXPLODE_CYLINDER:
			UTIL_BeamCylinder(pos, effect->explode_beam_radius, MODEL_INDEX(STRING(expSprite)),
				effect->explode_beam_frame, effect->explode_beam_fps,
				effect->explode_beam_life, effect->explode_beam_width,
				effect->explode_beam_noise, effect->explode_beam_color,
				effect->explode_beam_scroll);
			break;
		case EXPLODE_TORUS:
			UTIL_BeamTorus(pos, effect->explode_beam_radius, MODEL_INDEX(STRING(expSprite)),
				effect->explode_beam_frame, effect->explode_beam_fps,
				effect->explode_beam_life, effect->explode_beam_width,
				effect->explode_beam_noise, effect->explode_beam_color,
				effect->explode_beam_scroll);
			break;
		}
	}

	if (effect->explode_radius > 0 && effect->explode_damage > 0 && owner)
	{
		CBaseEntity* ownerEnt = owner;
		float radius = effect->explode_radius;
		float dmg = effect->explode_damage;

		if (friendlyFire)
		{
			// set class of all players to opposite of attacker, just until after we call RadiusDamage
			vector<CBaseEntity*> victims;
			vector<int> oldClassify;
			CBaseEntity* victim = NULL;
			do {
				victim = UTIL_FindEntityByClassname(victim, "player");
				if (victim)
				{
					victims.push_back(victim);
					oldClassify.push_back(victim->Classify());
					victim->SetClassification(CLASS_ALIEN_MILITARY);
				}
			} while (victim);

			ownerEnt->SetClassification(CLASS_PLAYER);

			RadiusDamage(pos, ownerEnt->pev, ownerEnt->pev, dmg, radius, 0, effect->damageType());

			for (int i = 0; i < victims.size(); i++)
				victims[i]->SetClassification(oldClassify[i]);
		}
		else
		{
			RadiusDamage(pos, ownerEnt->pev, ownerEnt->pev, dmg, radius, 0, effect->damageType());
		}
	}

	if (effect->explode_smoke_spr)
		g_Scheduler.SetTimeout(UTIL_Smoke, effect->explode_smoke_delay, pos,
			MODEL_INDEX(STRING(effect->explode_smoke_spr)), smokeScale, smokeFps);
}

void custom_effect(Vector pos, EHANDLE h_effect, EHANDLE creator, EHANDLE target,
	EHANDLE owner, Vector vDir, int flags, DecalTarget* dt)
{
	CWeaponCustomEffect* effect = (CWeaponCustomEffect*)h_effect.GetEntity();

	if (!effect || !effect->valid)
		return;

	if (!dt)
	{
		static DecalTarget temp; // TODO: should be unique per call
		temp = getProjectileDecalTarget(creator.GetEntity() ? creator.GetEntity() : NULL, !creator.GetEntity() ? pos : Vector(0, 0, 0), 32);
		dt = &temp;
	}

	bool delayFinished = (flags & FL_EFFECT_DELAY_FINISHED) != 0;
	bool friendlyFire = (flags & FL_EFFECT_FRIENDLY_FIRE) != 0;
	if (effect->delay > 0 && !delayFinished)
	{
		g_Scheduler.SetTimeout(custom_effect, effect->delay, pos, h_effect, creator, target, owner,
			vDir, flags | FL_EFFECT_DELAY_FINISHED, dt);
		return;
	}

	// velocity for explosion-type effects
	Vector vel = delayFinished || !creator.GetEntity() ? Vector(0, 0, 0) : creator.GetEntity()->pev->velocity;
	bool inWater = UTIL_PointContents(pos) == CONTENTS_WATER;
	Vector dir = !creator.GetEntity() ? dt->tr.vecPlaneNormal : creator.GetEntity()->pev->velocity.Normalize();

	if ((effect->pev->spawnflags & FL_EFFECT_EXPLOSION) != 0)
	{
		Vector exp_pos = pos;
		if (dt->ent)
			exp_pos = exp_pos + dt->tr.vecPlaneNormal * effect->explode_offset;
		custom_explosion(exp_pos, vel, effect, dt->pos, dt->ent, owner, inWater, friendlyFire);
	}
	if ((effect->pev->spawnflags & FL_EFFECT_LIGHTS) != 0)
	{
		int l_size = int(effect->explode_light_adv.x);
		int l_life = int(effect->explode_light_adv.y);
		int l_decay = int(effect->explode_light_adv.z);
		if (l_size > 0 && l_life > 0)
		{
			UTIL_DLight(pos, l_size, effect->explode_light_color.rgb(), l_life, l_decay);
			if (creator.GetEntity())
				UTIL_ELight(creator->entindex(), 0, pos, l_size * 10, effect->explode_light_color,
					l_life, l_decay);
		}

		int l_size2 = int(effect->explode_light_adv2.x);
		int l_life2 = int(effect->explode_light_adv2.y);
		int l_decay2 = int(effect->explode_light_adv2.z);
		if (l_size2 > 0)
		{
			UTIL_DLight(pos, l_size2, effect->explode_light_color2.rgb(), l_life2, l_decay2);
			if (creator.GetEntity())
				UTIL_ELight(creator->entindex(), 0, pos, l_size2 * 10, effect->explode_light_color2, l_life2, l_decay2);
		}
	}
	if ((effect->pev->spawnflags & FL_EFFECT_SPARKS) != 0)
	{
		UTIL_Sparks(pos);
	}
	if ((effect->pev->spawnflags & FL_EFFECT_RICOCHET) != 0)
	{
		UTIL_Ricochet(pos, 0);
	}
	if ((effect->pev->spawnflags & FL_EFFECT_TARBABY) != 0)
	{
		ALERT(at_error, "te_tarexplosion effect not implemented");
		//te_tarexplosion(pos);
	}
	if ((effect->pev->spawnflags & FL_EFFECT_TARBABY2) != 0)
	{
		ALERT(at_error, "te_explosion2 effect not implemented");
		//te_explosion2(pos);
	}
	if ((effect->pev->spawnflags & FL_EFFECT_BURST) != 0)
	{
		ALERT(at_error, "te_particlebust effect not implemented");
		//te_particlebust(pos, effect->burst_radius, effect->burst_color, effect->burst_life);
	}
	if ((effect->pev->spawnflags & FL_EFFECT_LAVA) != 0)
	{
		ALERT(at_error, "te_lavasplash effect not implemented");
		//te_lavasplash(pos);
	}
	if ((effect->pev->spawnflags & FL_EFFECT_TELEPORT) != 0)
	{
		ALERT(at_error, "te_teleport effect not implemented");
		//te_teleport(pos);
	}
	if (effect->glow_spr)
	{
		ALERT(at_error, "te_glowsprite effect not implemented");
		//te_glowsprite(pos, effect->glow_spr, effect->glow_spr_life, effect->glow_spr_scale, effect->glow_spr_opacity);
	}
	if (effect->spray_count > 0 && effect->spray_sprite > 0)
	{
		UTIL_SpriteSpray(pos, dir, effect->spray_sprite, effect->spray_count, effect->spray_speed, effect->spray_rand);
	}
	if (effect->implode_count > 0)
	{
		ALERT(at_error, "te_implosion effect not implemented");
		//te_implosion(pos, effect->implode_radius, effect->implode_count, effect->implode_life);
	}
	if (effect->rico_part_count > 0)
	{
		ALERT(at_error, "te_spritetrail effect not implemented");
		/*
		te_spritetrail(pos, pos + dt->tr.vecPlaneNormal, effect->rico_part_spr,
			effect->rico_part_count, 0, effect->rico_part_scale,
			effect->rico_part_speed, effect->rico_part_speed / 2);
		*/
	}
	if (effect->blood_stream != 0)
	{
		int stream_power = effect->blood_stream;
		Vector bdir = vDir;
		if (bdir == Vector())
			bdir = dir;
		if (stream_power < 0)
		{
			stream_power = -stream_power;
			bdir = bdir * -1;
		}
		int bcolor = 0;
		if (target)
		{
			CBaseEntity* targetEnt = target;
			bcolor = targetEnt->BloodColor();
			if (bcolor == BloodColorHuman()) // 247
				bcolor = 222; // the enum val is wrong
		}

		UTIL_BloodStream(pos, bdir, bcolor, stream_power);
	}
	if (effect->rico_trace_count > 0)
	{
		ALERT(at_error, "te_streaksplash effect not implemented");
		//te_streaksplash(pos, dt->tr.vecPlaneNormal, effect->rico_trace_color,
		//	effect->rico_trace_count, effect->rico_trace_speed, effect->rico_trace_rand);
	}
	if (effect->rico_decal != DECAL_NONE && dt->ent)
	{
		const char* decal = getBulletDecalOverride(dt->ent, getDecal(effect->rico_decal));
		if ((effect->pev->spawnflags & FL_EFFECT_GUNSHOT_RICOCHET) != 0)
			UTIL_GunshotDecal(dt->ent->entindex(), pos, DECAL_INDEX(decal));
		else
			UTIL_Decal(dt->ent->entindex(), pos, DECAL_INDEX(decal));
	}

	if (effect->explode_gibs > 0 && effect->explode_gib_mdl)
	{
		UTIL_BreakModel(pos, Vector(2, 2, 2), dir * effect->explode_gib_speed, effect->explode_gib_rand,
			effect->explode_gib_mdl, effect->explode_gibs, 5, effect->explode_gib_mat | effect->explode_gib_effects);
	}
	if (effect->explode_bubbles > 0 && (inWater || (effect->pev->spawnflags & FL_EFFECT_BUBBLES_IN_AIR) != 0))
	{
		Vector mins = pos + effect->explode_bubble_mins;
		Vector maxs = pos + effect->explode_bubble_maxs;
		float height = (maxs.z - mins.z) * 2.0f;
		if (inWater)
			height = UTIL_WaterLevel(pos, pos.z, pos.z + 1024) - mins.z;
		string_t spr = effect->explode_bubble_spr;
		if (!spr)
			spr = MAKE_STRING("sprites/bubble.spr");
		int count = effect->explode_bubbles;
		float speed = effect->explode_bubble_speed;
		g_Scheduler.SetTimeout(UTIL_Bubbles, effect->explode_bubble_delay, mins, maxs, count, height,
			MODEL_INDEX(STRING(spr)), speed);
	}
	if (effect->shake_radius > 0)
	{
		UTIL_ScreenShake(pos, effect->shake_amp, effect->shake_freq, effect->shake_time, effect->shake_radius);
	}

	WeaponSound* rico_snd = effect->getRandomSound();
	if (rico_snd)
		rico_snd->play(pos, CHAN_STATIC);

	if (effect->next_effect)
		custom_effect(pos, effect->next_effect, creator, target, owner, vDir, flags & ~FL_EFFECT_DELAY_FINISHED, dt);
}


CWeaponCustomEffect* loadEffectSettings(CWeaponCustomEffect* effect, string_t name)
{
	if (effect && effect->valid)
		return effect;

	string_t searchStr = effect ? effect->name : name;
	CBaseEntity* ent = UTIL_FindEntityByTargetname(NULL, STRING(searchStr));
	if (ent)
	{
		CWeaponCustomEffect* ef = (CWeaponCustomEffect*)ent;
		ef->loadExternalSoundSettings();
		ef->valid = true;
		ef->name = searchStr;

		*effect = *ef; // TODO: why duplicate?

		ef->loadExternalEffectSettings(); // load chained effects
		return ef;
	}
	else if (searchStr)
	{
		ALERT(at_error, "WEAPON_CUSTOM: Failed to find weapon_custom_effect '%s'", STRING(searchStr));
	}
	return effect;
}

void CWeaponCustomEffect::KeyValue(KeyValueData* pkvd)
{
	if (HandleKv(pkvd, "explosion_style"))   explosion_style = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "delay"))       delay = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "blood_stream"))      blood_stream = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_radius"))    explode_radius = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_dmg"))       explode_damage = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_offset"))    explode_offset = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_spr"))       explode_spr = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_water_spr")) explode_water_spr = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_spr_scale")) explode_spr_scale = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_spr_fps"))   explode_spr_fps = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_smoke_spr")) explode_smoke_spr = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_smoke_spr_scale")) explode_smoke_spr_scale = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_smoke_spr_fps")) explode_smoke_spr_fps = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_smoke_delay")) explode_smoke_delay = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_light_color")) explode_light_color = UTIL_ParseRGBA(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_light_adv"))   explode_light_adv = UTIL_ParseVector(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_light_color2")) explode_light_color2 = UTIL_ParseRGBA(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_light_adv2"))   explode_light_adv2 = UTIL_ParseVector(pkvd->szValue);

	else if (HandleKv(pkvd, "explode_beam_width"))  explode_beam_width = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_beam_life"))  explode_beam_life = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_beam_noise"))  explode_beam_noise = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_beam_frame"))  explode_beam_frame = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_beam_fps"))  explode_beam_fps = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_beam_scroll"))  explode_beam_scroll = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_beam_radius"))  explode_beam_radius = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_beam_color"))  explode_beam_color = UTIL_ParseRGBA(pkvd->szValue);

	else if (HandleKv(pkvd, "implode_count"))  implode_count = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "implode_radius")) implode_radius = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "implode_life"))   implode_life = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "shake_radius")) shake_radius = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "shake_amp"))    shake_amp = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "shake_freq"))   shake_freq = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "shake_time"))   shake_time = atof(pkvd->szValue);

	else if (HandleKv(pkvd, "burst_life"))   burst_life = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "burst_radius")) burst_radius = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "burst_color"))  burst_color = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "spray_count"))  spray_count = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "spray_sprite")) spray_sprite = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "spray_speed"))  spray_speed = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "spray_rand"))   spray_rand = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "explode_gibs"))      explode_gibs = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_gib_speed")) explode_gib_speed = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_gib_model")) explode_gib_mdl = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_gib_mat"))   explode_gib_mat = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_gib_rand"))  explode_gib_rand = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_gib_effects"))explode_gib_effects = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "glow_spr"))         glow_spr = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "glow_spr_scale"))   glow_spr_scale = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "glow_spr_life"))    glow_spr_life = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "glow_spr_opacity")) glow_spr_opacity = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "damage_type"))  damage_type = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "damage_type2"))  damage_type2 = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "gib_type"))  gib_type = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "sounds"))        parseSounds(pkvd, sounds);

	else if (HandleKv(pkvd, "rico_decal"))       rico_decal = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "rico_part_spr"))    rico_part_spr = ALLOC_STRING(pkvd->szValue);
	else if (HandleKv(pkvd, "rico_part_count"))  rico_part_count = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "rico_part_scale"))  rico_part_scale = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "rico_part_speed"))  rico_part_speed = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "rico_trace_count")) rico_trace_count = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "rico_trace_speed")) rico_trace_speed = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "rico_trace_rand"))  rico_trace_rand = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "rico_trace_color")) rico_trace_color = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "explode_bubbles"))       explode_bubbles = atoi(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_bubble_mins"))   explode_bubble_mins = UTIL_ParseVector(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_bubble_maxs"))   explode_bubble_maxs = UTIL_ParseVector(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_bubble_delay"))  explode_bubble_delay = atof(pkvd->szValue);
	else if (HandleKv(pkvd, "explode_bubble_spr"))    explode_bubble_spr = ALLOC_STRING(pkvd->szValue);

	else if (HandleKv(pkvd, "rico_scale"))    rico_scale = atoi(pkvd->szValue);

	else if (HandleKv(pkvd, "next_effect")) next_effect_str = ALLOC_STRING(pkvd->szValue);

	else CBaseEntity::KeyValue(pkvd);
}

void CWeaponCustomEffect::Spawn()
{
	Precache();
}

void CWeaponCustomEffect::loadExternalSoundSettings()
{
	loadSoundSettings(sounds);
}

void CWeaponCustomEffect::loadExternalEffectSettings()
{
	if (next_effect_loaded)
		return; // fix recursion crash
	next_effect_loaded = true;
	next_effect = loadEffectSettings((CWeaponCustomEffect*)next_effect.GetEntity(), next_effect_str);
}

WeaponSound* CWeaponCustomEffect::getRandomSound()
{
	if (sounds.size == 0)
		return NULL;
	int randIdx = RANDOM_LONG(0, sounds.size - 1);
	return &sounds.data[randIdx];
}

void CWeaponCustomEffect::PrecacheSound(string_t sound)
{
	if (sound) {
		EALERT(at_aiconsole, "Precaching sound: %s\n", STRING(sound));
		PRECACHE_SOUND(STRING(sound));
	}
}

int CWeaponCustomEffect::PrecacheModel(string_t model)
{
	if (model) {
		EALERT(at_aiconsole, "Precaching model: %s\n", STRING(model));
		return PRECACHE_MODEL(STRING(model));
	}
	return -1;
}

int CWeaponCustomEffect::damageType()
{
	return damage_type | damage_type2 | gib_type;
}

void CWeaponCustomEffect::Precache()
{
	for (int i = 0; i < sounds.size; i++)
		PrecacheSound(sounds.data[i].file);

	PrecacheModel(explode_spr);
	PrecacheModel(explode_smoke_spr);
	PrecacheModel(explode_gib_mdl);
	PrecacheModel(rico_part_spr);
	PrecacheModel(explode_water_spr);
	PrecacheModel(explode_bubble_spr);
	PrecacheModel(glow_spr);
	PrecacheModel(spray_sprite);
}

void CWeaponCustomEffect::Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float value)
{
	EHANDLE h_ent = edict();
	UTIL_MakeVectors(pev->angles);
	custom_effect(pev->origin, h_ent, h_ent, h_ent, h_ent, gpGlobals->v_forward, 0);
}

LINK_ENTITY_TO_CLASS(weapon_custom_effect, CWeaponCustomEffect)