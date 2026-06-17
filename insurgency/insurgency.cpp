#include "extdll.h"
#include "util.h"
#include "weapons.h"
#include "PluginHooks.h"

extern "C" int DLLEXPORT PluginInit() {
	static HLCOOP_PLUGIN_HOOKS g_hooks;
	return RegisterPlugin(&g_hooks);
}

extern "C" void DLLEXPORT PluginExit() {
	// nothing to clean up
}