#include "extdll.h"
#include "util.h"
#include "CTriggerChangeSky.h"

HLCOOP_PLUGIN_HOOKS g_hooks;

extern "C" int DLLEXPORT PluginInit() {
	return RegisterPlugin(&g_hooks);
}

extern "C" void DLLEXPORT PluginExit() {
	// nothing to clean up
}

LINK_ENTITY_TO_CLASS(trigger_changesky2, CTriggerChangeSky)