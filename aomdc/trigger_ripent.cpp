#include "extdll.h"
#include "util.h"
#include "CPointEntity.h"

class CTriggerRipent : public CPointEntity
{
	void Spawn(void)
	{
		LoadEntFile();
		UTIL_Remove(this);
	}

	char* loadEntFileDat(const char* prefix, int& len) {
		const char* mapName = STRING(gpGlobals->mapname);
		if (strlen(mapName) < strlen("aomdc_a")) {
			EALERT(at_error, "map name missing aomdc_ prefix.\n");
		}
		mapName = mapName + strlen("aomdc_");

		const char* entfile = UTIL_VarArgs("maps/aomdc_%s%s.ent", prefix, mapName);

		EALERT(at_console, "Loading ending %s ents from file: %s\n", prefix, entfile);

		char* data = (char*)UTIL_LoadFile(entfile, &len);

		if (!data) {
			EALERT(at_error, "Failed to load ent file: %s\n", entfile);
			return NULL;
		}

		return data;
	}

	void LoadEntFile() {
		const char* prefix = "3";
		char* data = NULL;
		int len = 0;

		if (gGlobalState.EntityGetState(MAKE_STRING("ending1")) == GLOBAL_ON) {
			data = loadEntFileDat("1", len);
		}
		else if (gGlobalState.EntityGetState(MAKE_STRING("ending2")) == GLOBAL_ON) {
			data = loadEntFileDat("2", len);
		}
		else if (gGlobalState.EntityGetState(MAKE_STRING("ending3")) == GLOBAL_ON) {
			data = loadEntFileDat("3", len);
		}
		else if (gGlobalState.EntityGetState(MAKE_STRING("ending4")) == GLOBAL_ON) {
			data = loadEntFileDat("4", len);
		}
		
		if (!data) {
			ALERT(at_error, "Failed to load ending global state. Falling back to ending 3.\n");
			data = loadEntFileDat("3", len); // ending 3 should work for every map if something went wrong
		}

		if (!data) {
			return;
		}

		std::vector<StringMap> ents;
		Bsp::parseEntities(data, len, ents);
		delete[] data;

		int spawnedEnts = 0;

		for (StringMap& def : ents) {
			const char* cname = def.get("classname");

			if (!cname || !strcmp(cname, "worldspawn")) {
				continue;
			}

			CBaseEntity* ent = CBaseEntity::Create(cname, g_vecZero, g_vecZero, true, NULL, def);
			spawnedEnts++;
		}

		EALERT(at_console, "Spawned %d entities\n", spawnedEnts);
	}
};

LINK_ENTITY_TO_CLASS(trigger_ripent, CTriggerRipent)