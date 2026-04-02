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

//extern ItemInfo g_weapon_base_info;

HLCOOP_PLUGIN_HOOKS g_hooks;

HOOK_RETURN_DATA MapInit() {
	//g_weapon_base_info = UTIL_RegisterWeapon("weapon_custom");

	return HOOK_CONTINUE;
}

HOOK_RETURN_DATA MapActivate() {
	g_map_activated = true;

	// Hook up weapon_custom with weapon_custom_shoot
	HashMap<EHANDLE>::iterator_t iter;
	while (custom_weapons.iterate(iter)) {
		CWeaponCustomConfig* wep = (CWeaponCustomConfig*)iter.value->GetEntity();
		if (wep)
			wep->link_shoot_settings();
	}

	// Hook up ambient_generic with weapon_custom_shoot
	HashMap<EHANDLE>::iterator_t iter2;
	while (custom_weapon_shoots.iterate(iter2)) {
		CWeaponCustomShoot* shoot = (CWeaponCustomShoot*)iter2.value->GetEntity();
		shoot->loadExternalSoundSettings();
		shoot->loadExternalEffectSettings();
	}

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
