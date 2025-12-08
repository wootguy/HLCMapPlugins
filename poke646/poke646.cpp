#include "extdll.h"
#include "util.h"
#include "weapons.h"
#include "PluginHooks.h"

extern ItemInfo g_cmlwbr_info;
extern ItemInfo g_par21_info;
extern ItemInfo g_sawedoff_info;
extern ItemInfo g_bradnailer_info;
extern ItemInfo g_nailgun_info;
extern ItemInfo g_heaterpipe_info;
extern ItemInfo g_leadpipe_info;

HLCOOP_PLUGIN_HOOKS g_hooks;

HOOK_RETURN_DATA MapInit() {
	g_heaterpipe_info = UTIL_RegisterWeapon("weapon_heaterpipe");
	g_cmlwbr_info = UTIL_RegisterWeapon("weapon_cmlwbr");
	g_bradnailer_info = UTIL_RegisterWeapon("weapon_bradnailer");
	g_nailgun_info = UTIL_RegisterWeapon("weapon_nailgun");
	g_par21_info = UTIL_RegisterWeapon("weapon_par21");
	g_sawedoff_info = UTIL_RegisterWeapon("weapon_sawedoff");

	g_weaponRemapHL.put("weapon_9mmAR", "weapon_par21");
	g_weaponRemapHL.put("weapon_shotgun", "weapon_sawedoff");

	UTIL_RegisterEquipmentEntity("weapon_leadpipe");

	return HOOK_CONTINUE;
}

extern "C" int DLLEXPORT PluginInit() {
	g_hooks.pfnMapInit = MapInit;

	return RegisterPlugin(&g_hooks);
}

extern "C" void DLLEXPORT PluginExit() {
	// nothing to clean up
}