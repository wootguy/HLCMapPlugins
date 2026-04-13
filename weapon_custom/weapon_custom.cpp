#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"
#include "const_wc.h"
#include "weapon_custom.h"
#include "WeaponSound.h"
#include "weapons.h"
#include "PluginHooks.h"
#include "CWeaponCustomConfig.h"
#include "CWeaponCustomShoot.h"

bool debug_mode = false;
bool g_map_activated = false;
bool g_mapinit_finished = false;

// WeaponCustomBase will read this to get weapon_custom settings
// Also let's us know which weapon slots are used (Auto weapon slot position depends on this)
HashMap<EHANDLE> custom_weapons;
HashMap<EHANDLE> custom_ammos;
HashMap<EHANDLE> custom_weapon_shoots;
HashMap<EHANDLE> custom_weapon_effects;

vector<const char*> g_panim_refs = {
	"crowbar", "gren", "trip", "onehanded", "python", "shotgun", "gauss", "mp5",
	"rpg", "egon", "squeak", "hive", "bow", "minigun", "uzis", "m16", "sniper", "saw"
};

vector<const char*> g_ammo_types = {
	"buckshot", "health", "556", "m40a1", "argrenades", "357", "9mm", "shock charges", "sporeclip",
	"uranium", "rockets", "bolts", "trip mine", "satchel charge", "hand grenade", "snarks", "hornets"
};

HashMap<int> g_wep_name_info_idx;
ItemInfo g_wep_info[MAX_WEAPONS];
int g_wep_info_count = 1; // using 0 as an invalid index


HLCOOP_PLUGIN_HOOKS g_hooks;

HOOK_RETURN_DATA MapInit() {
	g_map_activated = false;
	g_mapinit_finished = false;

	static StringSet configEnts = {
		"weapon_custom", "weapon_custom_bullet", "weapon_custom_beam", "weapon_custom_melee",
		"weapon_custom_projectile", "weapon_custom_effect", "weapon_custom_user_effect",
		"weapon_custom_sound"
	};

	// spawn ammo entities now so weapons can get their max capacity
	for (StringMap& kv : g_bsp.ents) {
		const char* cname = kv.get("classname");

		if (!cname || strcmp(cname, "weapon_custom_ammo")) {
			continue;
		}

		CBaseEntity::Create(cname, g_vecZero, g_vecZero, true, 0, kv);
	}

	// all weapon configs must be set up now or else the engine won't spawn map weapons properly
	for (StringMap& kv : g_bsp.ents) {
		const char* cname = kv.get("classname");

		if (!cname || !configEnts.hasKey(cname)) {
			continue;
		}

		CBaseEntity::Create(cname, g_vecZero, g_vecZero, true, 0, kv);
	}

	// Hook up weapon_custom with weapon_custom_shoot
	HashMap<EHANDLE>::iterator_t iter;
	while (custom_weapons.iterate(iter)) {
		CWeaponCustomConfig* wep = (CWeaponCustomConfig*)iter.value->GetEntity();
		if (wep) {
			wep->link_shoot_settings();
		}
	}

	// Hook up ambient_generic with weapon_custom_shoot
	HashMap<EHANDLE>::iterator_t iter2;
	while (custom_weapon_shoots.iterate(iter2)) {
		CWeaponCustomShoot* shoot = (CWeaponCustomShoot*)iter2.value->GetEntity();
		shoot->loadExternalSoundSettings();
		shoot->loadExternalEffectSettings();
	}

	g_mapinit_finished = true;

	return HOOK_CONTINUE;
}

HOOK_RETURN_DATA MapActivate() {
	g_map_activated = true;
	return HOOK_CONTINUE;
}

extern "C" int DLLEXPORT PluginInit() {
	g_hooks.pfnMapInit = MapInit;
	g_hooks.pfnMapStart = MapActivate;

	return RegisterPlugin(&g_hooks);
}

extern "C" void DLLEXPORT PluginExit() {
	// nothing to clean up
}
