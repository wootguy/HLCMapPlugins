#include "extdll.h"
#include "util.h"
#pragma once
#include "const_wc.h"
#include <string>
#include <rgb.h>
#include "WeaponSound.h"
#include "util_wc.h"

using namespace std;

struct BeamOptions
{
	int type;
	int width;
	int noise;
	int scrollRate;
	float time;
	string_t sprite;
	RGBA color;

	RGBA alt_color;
	int alt_width;
	int alt_noise;
	int alt_scrollRate;
	int alt_mode;
	float alt_time;
};

struct ProjectileOptions
{
	int type = PROJECTILE_CUSTOM;
	int world_event = PROJ_ACT_ATTACH;
	int monster_event = PROJ_ACT_ATTACH;
	float speed = 0;
	float life = 0;
	float elasticity = 0.8; // percentage of reflected velocity
	float gravity = 0.0; // percentage of normal gravity
	float air_friction = 0;
	float water_friction = 0;
	float size = 0.001;		  // hull size (all dimensions)
	Vector dir = Vector(0, 0, 1);
	string_t entity_class; // custom projectile entity
	string_t model;
	WeaponSound move_snd;

	string_t sprite;
	RGBA sprite_color;
	float sprite_scale;

	Vector angles;
	Vector avel;
	Vector offset;
	Vector player_vel_inf;

	int follow_mode = FOLLOW_NONE;
	float follow_radius = 0.0f;
	float follow_angle = 30.0f;
	Vector follow_time;

	string_t trail_spr;
	int trail_sprId = 2; // remove me
	int trail_life;
	int trail_width;
	RGBA trail_color;
	float trail_effect_freq;
	float bounce_effect_delay;
};

struct AmmoDrop
{
	string_t cname;
	int dropAmt = 0;
};

extern bool debug_mode;
extern bool g_map_activated;

// WeaponCustomBase will read this to get weapon_custom settings
// Also let's us know which weapon slots are used (Auto weapon slot position depends on this)
extern HashMap<EHANDLE> custom_weapons;
extern HashMap<EHANDLE> custom_ammos;
extern HashMap<EHANDLE> custom_weapon_shoots;
extern HashMap<EHANDLE> custom_weapon_effects;

extern vector<const char*> g_panim_refs;
extern vector<const char*> g_ammo_types;