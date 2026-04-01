#include "extdll.h"
#include "util.h"
#include "weapons.h"
#include "PluginHooks.h"

extern ItemInfo g_fiveseven_info;
extern ItemInfo g_colt_info;
extern ItemInfo g_glocks_info;
extern ItemInfo g_usp_info;
extern ItemInfo g_sawedoff_info;

HLCOOP_PLUGIN_HOOKS g_hooks;

HOOK_RETURN_DATA MapInit() {
	g_fiveseven_info = UTIL_RegisterWeapon("weapon_fiveseven");
	g_colt_info = UTIL_RegisterWeapon("weapon_colt");
	g_glocks_info = UTIL_RegisterWeapon("weapon_dualglock");
	g_usp_info = UTIL_RegisterWeapon("weapon_usp");
	g_sawedoff_info = UTIL_RegisterWeapon("weapon_sawedoff");

	return HOOK_CONTINUE;
}

extern "C" int DLLEXPORT PluginInit() {
	g_hooks.pfnMapInit = MapInit;

	return RegisterPlugin(&g_hooks);
}

extern "C" void DLLEXPORT PluginExit() {
	// nothing to clean up
}