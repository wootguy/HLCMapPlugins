#pragma once
#include "extdll.h"
#include "util.h"
#include "const_wc.h"
#include <string>
#include <rgb.h>
#include "WeaponSound.h"
#include "util_wc.h"
#include "weapons.h"
#include "custom_weapon.h"

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

struct AmmoDrop
{
	string_t cname;
	int dropAmt = 0;
};

extern bool debug_mode;
extern bool g_map_activated;
extern bool g_mapinit_finished;

// WeaponCustomBase will read this to get weapon_custom settings
// Also let's us know which weapon slots are used (Auto weapon slot position depends on this)
extern HashMap<EHANDLE> custom_weapons;
extern HashMap<EHANDLE> custom_ammos;
extern HashMap<EHANDLE> custom_weapon_shoots;
extern HashMap<EHANDLE> custom_weapon_effects;

extern vector<const char*> g_panim_refs;
extern vector<const char*> g_ammo_types;

extern HashMap<int> g_wep_name_info_idx; // maps a weapon class name to an index in g_wep_info
extern ItemInfo g_wep_info[MAX_WEAPONS];
extern int g_wep_info_count;

extern "C" DLLEXPORT void weapon_custom_base(entvars_t* pev);