#include "extdll.h"
#include "util.h"
#include "CBasePlayerWeapon.h"
#include "Scheduler.h"

void print_hashmap(BaseHashMap& hmap) {
	hmap.print();
}

using namespace std;

namespace bspguy {
	bool noscript = false; // true if this script shouldn't be used but is loaded anyway
	int survival_is_restarting = 0;
	
	StringSet no_delete_ents; // entity classes that don't work right if spawned late
	StringSet map_loaded;
	StringSet map_cleaned;
	vector<string> map_order;
	int current_map_idx;
	
	void spawnMapEnts(string mapName);
	void deleteMapEnts(string mapName, bool invertFilter, bool spawnsOnly);
	void restart_survival_section(bool force_restart);

	string getCustomStringKeyvalue(CBaseEntity* ent, const char* keyName) {
		CKeyValue keyvalue = ent->GetCustomKeyValue(keyName);
		if (keyvalue.exists()) {
			return STRING(keyvalue.sVal);
		}
		return "";
	}

	Vector getCustomVectorKeyvalue(CBaseEntity* ent, const char* keyName) {
		CKeyValue keyvalue = ent->GetCustomKeyValue(keyName);
		if (keyvalue.exists()) {
			return keyvalue.vVal;
		}
		return g_vecZero;
	}

	int getCustomIntegerKeyvalue(CBaseEntity* ent, const char* keyName) {
		CKeyValue keyvalue = ent->GetCustomKeyValue(keyName);
		if (keyvalue.exists()) {
			return keyvalue.iVal;
		}
		return 0;
	}

	float getCustomFloatKeyvalue(CBaseEntity* ent, const char* keyName) {
		CKeyValue keyvalue = ent->GetCustomKeyValue(keyName);
		if (keyvalue.exists()) {
			return keyvalue.fVal;
		}
		return 0;
	}

	vector<float> rotationMatrix(Vector axis, float angle)
	{
		angle = angle * M_PI / 180.0; // convert to radians
		axis = axis.Normalize();
		float s = sin(angle);
		float c = cos(angle);
		float oc = 1.0 - c;

		vector<float> mat = {
			oc * axis.x * axis.x + c,          oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0,
			oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c,          oc * axis.y * axis.z - axis.x * s, 0.0,
			oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c,			 0.0,
			0.0,                               0.0,                               0.0,								 1.0
		};
		return mat;
	}

	// multiply a matrix with a vector (assumes w component of vector is 1.0f) 
	Vector matMultVector(vector<float> rotMat, Vector v)
	{
		Vector outv;
		outv.x = rotMat[0] * v.x + rotMat[4] * v.y + rotMat[8] * v.z + rotMat[12];
		outv.y = rotMat[1] * v.x + rotMat[5] * v.y + rotMat[9] * v.z + rotMat[13];
		outv.z = rotMat[2] * v.x + rotMat[6] * v.y + rotMat[10] * v.z + rotMat[14];
		return outv;
	}

	bool hasCustomKeyvalue(CBaseEntity* ent, const char* keyName) {
		CKeyValue keyvalue = ent->GetCustomKeyValue(keyName);
		return keyvalue.exists();
	}

	void delay_respawn() {
		ALERT(at_console, "Respawning everyone\n");
		UTIL_RespawnAllPlayers(true, true);
		survival_is_restarting = 0;
	}
	
	void delay_trigger(EHANDLE h_ent) {
		CBaseEntity* ent = h_ent;
		
		if (!ent)
			return;

		ent->Use(NULL, NULL, USE_TOGGLE);
	}
	
	void delay_fire_targets(string target) {
		ALERT(at_console, "Triggering: %s\n", target.c_str());
		FireTargets(target.c_str(), NULL, NULL, USE_TOGGLE);
	}
	
	void resetMapCleanStates() {
		ALERT(at_console, "Reset map clean states\n");
		map_cleaned.clear();
	}

	void load_map_no_repeat(string map) {
		if (map_loaded.hasKey(map.c_str())) {
			ALERT(at_console, "Map %s has already loaded. Ignoring mapload trigger.\n", map.c_str());
			return;
		}
		
		g_Scheduler.SetTimeout(delay_fire_targets, 0.0f, "bspguy_start_" + map);
		
		spawnMapEnts(map);
		
		for (int i = 0; i < map_order.size(); i++) {
			if (map_order[i] == map) {
				current_map_idx = i;
				break;
			}
		}
	}
	
	void clean_map_no_repeat(string map) {
		if (map_cleaned.hasKey(map.c_str())) {
			ALERT(at_console, "Map %s has already been cleaned.\n", map.c_str());
			return;
		}
		
		deleteMapEnts(map, false, false); // delete everything
	}

	void mapchange_internal(string thisMap, string nextMap, bool fastchange = false) {
		for (int i = 0; i < map_order.size(); i++) {
			if (map_order[i] == nextMap) {
				current_map_idx = i;
				break;
			}
		}

		UTIL_ClientPrintAll(print_center, UTIL_VarArgs("Entering %s\n", nextMap.c_str()));

		float extraDelay = fastchange ? 0.0f : 1.5f;

		if (thisMap != nextMap) {
			vector<string> loadedKeys;
			StringSet::iterator_t iter;
			while (map_loaded.iterate(iter)) {
				loadedKeys.push_back(iter.key);
			}

			for (string& key : loadedKeys) {
				if (map_cleaned.hasKey(key.c_str())) {
					ALERT(at_console, "Map %s has already been cleaned. Skipping mapchange clean.\n", iter.key);
					continue;
				}

				deleteMapEnts(key, false, true); // delete spawns immediately, in all previous levels
				g_Scheduler.SetTimeout(clean_map_no_repeat, 1.0f + extraDelay, key); // everything else
			}

			spawnMapEnts(nextMap);
			g_Scheduler.SetTimeout(delay_fire_targets, 0.5f + extraDelay, "bspguy_start_" + nextMap);
			g_Scheduler.SetTimeout(delay_respawn, 1.1f + extraDelay);
			g_Scheduler.SetTimeout(resetMapCleanStates, 1.1f + extraDelay);

		}
		else {
			// restarting the same level
			deleteMapEnts(thisMap, false, false);
			spawnMapEnts(nextMap);
			g_Scheduler.SetTimeout(delay_respawn, 0.5f + extraDelay);
		}
	}
	
	void mapchange(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
	{
		string thisMap = toLowerCase(getCustomStringKeyvalue(pCaller, "$s_bspguy_map_source"));
		string nextMap = toLowerCase(getCustomStringKeyvalue(pCaller, "$s_next_map"));
		
		if (thisMap == "" || nextMap == "") {
			ALERT(at_console, "ERROR: bspguy_mapchange called by %s which is missing $s_bspguy_map_source or $s_next_map", STRING(pCaller->pev->classname));
			return;
		}
		
		if (map_cleaned.hasKey(thisMap.c_str())) {
			ALERT(at_console, "Map %s has already been cleaned. Ignoring mapchange trigger.\n", thisMap.c_str());
			return;
		}
		if (map_loaded.hasKey(nextMap.c_str())) {
			ALERT(at_console, "Map %s has already loaded. Ignoring mapchange trigger.\n", nextMap.c_str());
			return;
		}
		
		mapchange_internal(thisMap, nextMap);
	}
	
	void mapload(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
	{
		string nextMap = toLowerCase(getCustomStringKeyvalue(pCaller, "$s_next_map"));
		load_map_no_repeat(nextMap);
	}
	
	void mapclean(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
	{
		string cleanMap = toLowerCase(getCustomStringKeyvalue(pCaller, "$s_bspguy_map_source"));
		clean_map_no_repeat(cleanMap);
	}
	
	void maprestart(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
	{
		g_Scheduler.SetTimeout(restart_survival_section, 0, true);
	}
	
	void bspguy(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
	{		
		string loadMap = toLowerCase(getCustomStringKeyvalue(pCaller, "$s_bspguy_load"));
		string cleanMap = toLowerCase(getCustomStringKeyvalue(pCaller, "$s_bspguy_clean"));
		string rotate = getCustomStringKeyvalue(pCaller, "$s_bspguy_rotate");
		string trigger = getCustomStringKeyvalue(pCaller, "$s_bspguy_trigger");
		
		if (loadMap.length() > 0) {
			load_map_no_repeat(loadMap);
		}
		
		if (cleanMap.length() > 0) {
			vector<string> maps = splitString(cleanMap, "+");
			for (int i = 0; i < maps.size(); i++) {
				clean_map_no_repeat(maps[i]);
			}
		}
		
		if (rotate.length() > 0 && pActivator  && pCaller ) {
			float rot = atof(rotate.c_str());
			
			Vector delta = pActivator->pev->origin - pCaller->pev->origin;
			vector<float> yawRotMat = rotationMatrix(Vector(0,0,-1), rot);
			pActivator->pev->velocity = matMultVector(yawRotMat, pActivator->pev->velocity);
			pActivator->pev->origin = pCaller->pev->origin + matMultVector(yawRotMat, delta);
			
			if (pActivator->IsPlayer()) {
				pActivator->pev->angles = pActivator->pev->v_angle;
				pActivator->pev->angles.y += rot;
				pActivator->pev->fixangle = 1;
			} else {
				pActivator->pev->angles.y += rot;
			}
		}
		
		if (trigger.length() > 0) {
			int triggerTypeSep = trigger.find("#");
			bool killed = false;
			
			if (triggerTypeSep != -1) {
				int triggerType = atoi(trigger.substr(triggerTypeSep+1).c_str());
				trigger = trigger.substr(0, triggerTypeSep);
				
				if (triggerType == 0) {
					useType = USE_OFF;
				} else if (triggerType == 1) {
					useType = USE_ON;
				} else if (triggerType == 2) {
					killed = true;
					
					CBaseEntity* ent = NULL;
					do {
						ent = UTIL_FindEntityByTargetname(ent, trigger.c_str());
						if (ent) {
							UTIL_Remove(ent);
						}
					} while (ent );
				}
			}
			
			if (!killed)
				FireTargets(trigger.c_str(), pActivator, pCaller, useType);
		}
	}
	
	bool isSpawnEntity(const char* cname) {
		static StringSet spawnEnts = {
			"info_player_deathmatch"
			"info_player_start"
			"info_player_dm2"
			"info_player_coop"
		};

		return spawnEnts.hasKey(cname);
	}
	
	void deleteMapEnts(string mapName, bool invertFilter, bool spawnsOnly) {
		mapName = toLowerCase(mapName);
		string infoEntName = "bspguy_info_" + mapName;
		CBaseEntity* mapchangeEnt = UTIL_FindEntityByTargetname(NULL, infoEntName.c_str());
		
		bool minMaxLoaded = false;
		Vector min, max;
		if (mapchangeEnt) {
			min = getCustomVectorKeyvalue(mapchangeEnt, "$v_min");
			max = getCustomVectorKeyvalue(mapchangeEnt, "$v_max");
			minMaxLoaded = true;
		} else {
			ALERT(at_console, "ERROR: Missing entity '%s'. Some entities may not be deleted in previous maps, && that can cause lag!\n", infoEntName.c_str());
		}
	
		CBaseEntity* ent = NULL;
		do {
			ent = UTIL_FindEntityByClassname(ent, "*");
			if (ent) {
				if (spawnsOnly && !isSpawnEntity(STRING(ent->pev->classname)))
					continue;
			
				if (ent->IsPlayer() || string(STRING(ent->pev->targetname)).find("bspguy") == 0) {
					continue;
				}
				
				// don't remove player items/weapons
				CBasePlayerWeapon* item = ent->GetWeaponPtr();
				if (item && item->m_hPlayer) {
					continue;
				}

				if (no_delete_ents.hasKey(STRING(ent->pev->classname))) {
					continue;
				}
				
				CKeyValue mapKeyvalue = ent->GetCustomKeyValue("$s_bspguy_map_source");
				if (mapKeyvalue.exists()) {
					string mapSource = toLowerCase(STRING(mapKeyvalue.sVal));
					if (invertFilter && mapSource == mapName) {
						continue;
					} else if (!invertFilter && mapSource != mapName) {
						continue;
					}
				} else if (minMaxLoaded) {
					Vector ori = ent->pev->origin;
					// probably a entity that spawned from a squadmaker || something
					// skip if it's outside the map boundaries
					bool outOfBounds = ori.x < min.x || ori.x > max.x || ori.y < min.y || ori.y > max.y || ori.z < min.z || ori.z > max.z;
					if ((!invertFilter && outOfBounds) || (invertFilter && !outOfBounds)) {
						continue;
					}
				}
				
				UTIL_Remove(ent);
			}
		} while (ent );
		
		if (!spawnsOnly) {
			map_loaded.del(mapName.c_str());
			map_cleaned.put(mapName.c_str());
			ALERT(at_console, "Cleaned section %s\n", mapName.c_str());
		} else {
			ALERT(at_console, "Disabled spawns in section %s\n", mapName.c_str());
		}
	}
	
	void spawnMapEnts(string mapName) {
		mapName = toLowerCase(mapName);
	
		vector<CBaseEntity*> activateEnts;

		for (int i = 0; i < g_bsp.ents.size(); i++) {
			const char* mapSource = g_bsp.ents[i].get("$s_bspguy_map_source");
			
			if (mapSource && mapSource == mapName) {
				const char* classname = g_bsp.ents[i].get("classname");
				
				if (no_delete_ents.hasKey(classname) || !strcmp(classname, "info_node") || !strcmp(classname, "info_node_air")) {
					continue;
				}
				
				CBaseEntity* ent = CBaseEntity::Create(classname, g_vecZero, g_vecZero, true, NULL, g_bsp.ents[i]);
				
				if (ent) {
					activateEnts.push_back(ent);
				}
			}
		}

		for (CBaseEntity* ent : activateEnts) {
			ent->Activate();
		}
		
		map_loaded.put(mapName.c_str());
		map_cleaned.del(mapName.c_str());
		ALERT(at_console, "Loaded section %s\n", mapName.c_str());
		ALERT(at_console, "Triggering: bspguy_init_%s\n", mapName.c_str());
		FireTargets(("bspguy_init_" + mapName).c_str(), NULL, NULL, USE_TOGGLE);
	}
	
	HOOK_RETURN_DATA MapInit() {		
		no_delete_ents.put("multi_manager"); // never triggers anything if spawned late
		no_delete_ents.put("path_track"); // messes up track_train if spawned late
		no_delete_ents.put("soundcache"); // plugin entity
		no_delete_ents.put("bodyque"); // preallocated corpse ents
		no_delete_ents.put("trigger_camera"); // thirdperson plugin entity

		return HOOK_CONTINUE;
	}
	
	// restart the current section, rather than the entire map
	void survival_restart_check() {
		/*
		if (!g_SurvivalMode.IsActive() || count_living_players() > 0) {
			survival_is_restarting = 0;
			return;
		}
		
		survival_is_restarting += 1;
		if (survival_is_restarting < 5 || survival_is_restarting > 5) {
			// Wait a few seconds to be sure a restart is coming.
			// The ForceSurvival plugin might respawn everyone instead.
			return;
		}
		
		UTIL_ClientPrintAll(print_chat, "This is a merged map. The current section will reload shortly.\n");	
		g_Scheduler.SetTimeout(restart_survival_section, 3, false);
		*/
	}
	
	int count_living_players() {
		int totalLiving = 0;
		
		for (int i = 1; i <= gpGlobals->maxClients; i++) {
			CBasePlayer* pPlayer = UTIL_PlayerByIndex(i);

			if (pPlayer && pPlayer->IsAlive()) {
				totalLiving++;
			}
		}
		
		return totalLiving;
	}
	
	void restart_survival_section(bool force_restart) {		
		if (count_living_players() > 0 && !force_restart) {
			return;
		}
		
		string thisMap = map_order[current_map_idx];
		string loadMap = "";
		
		for (int i = 0; i < map_order.size(); i++) {			
			if (map_loaded.hasKey(map_order[i].c_str())) {
				if (loadMap.length() == 0) {
					loadMap = map_order[i];
				}
				clean_map_no_repeat(map_order[i]);
			}
		}
		
		mapchange_internal(loadMap, loadMap, true);
	}
	
	HOOK_RETURN_DATA MapActivate() {
		string firstMapName;
		CBaseEntity* infoEnt = UTIL_FindEntityByTargetname(NULL, "bspguy_info");
		if (infoEnt) {
			firstMapName = getCustomStringKeyvalue(infoEnt, "$s_map0");
			current_map_idx = 0;
			
			for (int i = 0; i < 64; i++) {
				string mapName = toLowerCase(getCustomStringKeyvalue(infoEnt, UTIL_VarArgs("$s_map%d", i)));
				if (mapName.length() > 0)
					map_order.push_back(mapName);
				else
					break;
			}
			
			noscript = getCustomStringKeyvalue(infoEnt, "$s_noscript") == "yes";
		} else {
			ALERT(at_console, "ERROR: Missing entity 'bspguy_info'. bspguy script disabled!");
			return HOOK_CONTINUE;
		}
		
		if (noscript) {
			ALERT(at_console, "WARNING: this map was not intended to be used with the bspguy script!");
			return HOOK_CONTINUE;
		}
		
		if (firstMapName.length() == 0) {
			ALERT(at_console, "ERROR: bspguy_info entity has no $s_mapX keys. bspguy script disabled!");
			return HOOK_CONTINUE;
		}
		
		StringMap keys;
		keys.put("delay", "0");
		keys.put("m_iszScriptFile", "bspguy/bspguy");
		keys.put("m_iMode", "1"); // trigger
		
		keys.put("targetname", "bspguy_maprestart");
		keys.put("m_iszScriptFunctionName", "bspguy::maprestart");
		CBaseEntity::Create("trigger_script", g_vecZero, g_vecZero, true, NULL, keys);
		
		keys.put("targetname", "bspguy_mapchange");
		keys.put("m_iszScriptFunctionName", "bspguy::mapchange");
		CBaseEntity::Create("trigger_script", g_vecZero, g_vecZero, true, NULL, keys);
		
		keys.put("targetname", "bspguy_mapload");
		keys.put("m_iszScriptFunctionName", "bspguy::mapload");
		CBaseEntity::Create("trigger_script", g_vecZero, g_vecZero, true, NULL, keys);
		
		keys.put("targetname", "bspguy_mapclean");
		keys.put("m_iszScriptFunctionName", "bspguy::mapclean");
		CBaseEntity::Create("trigger_script", g_vecZero, g_vecZero, true, NULL, keys);
		
		keys.put("targetname", "bspguy");
		keys.put("m_iszScriptFunctionName", "bspguy::bspguy");
		CBaseEntity::Create("trigger_script", g_vecZero, g_vecZero, true, NULL, keys);
		
		// all entities in all sections are spawned by now. Delete everything except for the ents in the first section.
		// It may be a bit slow to spawn all ents at first, but that will ensure everything is precached
		deleteMapEnts(firstMapName, true, false);
		map_loaded.put(firstMapName.c_str());
		map_cleaned.clear();
		
		g_Scheduler.SetTimeout(delay_fire_targets, 0.0f, "bspguy_start_" + firstMapName);
		g_Scheduler.SetInterval(survival_restart_check, 1.0f, -1);

		return HOOK_CONTINUE;
	}

	void printMapSections(EHANDLE h_plr) {
		CBasePlayer* plr = (CBasePlayer*)(h_plr.GetEntity());
	
		UTIL_ClientPrint(plr, print_console, "Map sections:\n");	
		for (int i = 0; i < map_order.size(); i++) {
			string begin = i < 9 ? "     " : "    ";
			
			bool isLoaded = map_loaded.hasKey(map_order[i].c_str());
			bool isCleaned = map_cleaned.hasKey(map_order[i].c_str());
			
			string end = "\n";
			
			if (i == current_map_idx) {
				end = "    (LOADED + CURRENT)\n";
			}
			else if (isLoaded) {
				end = "    (LOADED)\n";
			} else if (isCleaned) {
				end = "    (CLEANED)\n";
			}
			
			UTIL_ClientPrint(plr, print_console, (begin + to_string(i+1) + ") " + map_order[i] + end).c_str());
		}
	}

	bool doCommand(CBasePlayer* plr, const CommandArgs& args) {
		bool isAdmin = AdminLevel(plr) >= ADMIN_YES;
		EHANDLE h_plr = plr ? plr->edict() : NULL;
		
		if (args.ArgC() >= 2)
		{
			if (args.ArgV(1) == "version") {
				UTIL_ClientPrint(plr, print_chat, "bspguy script v2\n");
			}
			if (args.ArgV(1) == "list") {
				printMapSections(h_plr);
			}
			if (args.ArgV(1) == "spawn") {
				if (!isAdmin) {
					UTIL_ClientPrint(plr, print_chat, "Only admins can use that command.\n");
					return true;
				}
				g_Scheduler.SetInterval(delay_respawn, 0.1, 25);
			}
			if (args.ArgV(1) == "mapchange") {
				if (!isAdmin) {
					UTIL_ClientPrint(plr, print_chat, "Only admins can use that command.\n");
					return true;
				}
				if (args.ArgC() >= 3) {
					string arg = args.ArgV(2);
					string thisMap = map_order[current_map_idx];
					string nextMap;
					for (int i = 0; i < map_order.size(); i++) {
						if (toLowerCase(arg) == toLowerCase(map_order[i])) {
							nextMap = arg;
							break;
						}
					}
					if (nextMap.length() == 0) {
						int idx = atoi(arg.c_str()) - 1;
						if (idx < map_order.size()) {
							nextMap = map_order[idx];
						}
					}
					if (nextMap.length() == 0) {
						UTIL_ClientPrint(plr, print_chat, "Invalid section name/number. See \"bspguy list\" output.\n");
					} else {
						mapchange_internal(thisMap, nextMap, true);
						g_Scheduler.SetTimeout(printMapSections, 1.2f, h_plr);
					}
				} else {
					if (current_map_idx >= int(map_order.size())-1) {
						UTIL_ClientPrint(plr, print_chat, "This is the last map section.\n");
					} else {
						string thisMap = map_order[current_map_idx];
						string nextMap = map_order[current_map_idx+1];
						
						mapchange_internal(thisMap, nextMap, true);
						g_Scheduler.SetTimeout(printMapSections, 1.2f, h_plr);
					}
				}
			}
		} else {			
			UTIL_ClientPrint(plr, print_console, "----------------------------------bspguy commands----------------------------------\n\n");
			UTIL_ClientPrint(plr, print_console, "Type \"bspguy list\" to list map sections.\n\n");
			UTIL_ClientPrint(plr, print_console, "Type \"bspguy spawn\" to test spawn points.\n\n");
			UTIL_ClientPrint(plr, print_console, "Type \"bspguy mapchange [name|number]\" to transition to a new map section.\n");
			UTIL_ClientPrint(plr, print_console, "    [name|number] = Optional. Map section name || number to load (as shown in \"bspguy list\")\n");
			UTIL_ClientPrint(plr, print_console, "\n-----------------------------------------------------------------------------------\n\n");
		}

		return false;
	}

	HOOK_RETURN_DATA EquipPlayerSpawn(CBasePlayer* plr);
}

HLCOOP_PLUGIN_HOOKS g_hooks;

extern "C" int DLLEXPORT PluginInit(int interfaceVersion) {
	g_hooks.pfnMapInit = bspguy::MapInit;
	g_hooks.pfnServerActivate = bspguy::MapActivate;
	g_hooks.pfnPlayerSpawn = bspguy::EquipPlayerSpawn;

	RegisterPluginCommand("bspguy", bspguy::doCommand, FL_CMD_CLIENT_CONSOLE | FL_CMD_SERVER);

	RegisterPluginEntCallback(bspguy::mapload);
	RegisterPluginEntCallback(bspguy::mapchange);
	RegisterPluginEntCallback(bspguy::mapclean);
	RegisterPluginEntCallback(bspguy::maprestart);
	RegisterPluginEntCallback(bspguy::bspguy);

	return RegisterPlugin(&g_hooks);
}

extern "C" void DLLEXPORT PluginExit() {}

