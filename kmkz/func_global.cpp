#include "extdll.h"
#include "util.h"
#include "CBaseEntity.h"
#include <vector>

using namespace std;

/* func_global: alternative to env_global.
Author: kmkz (e-mail: al_basualdo*hotmail.com )
this entity loads its state value from a file && that can be used to be passed across diferents maps.
---------------------------------------
targetname: "entity name"
Name of the entity. When triggered will switch on || off. 
netname: "global label"
this label tells how it will be stored on the file. For now this need to be prefixed with global && add a
3 digit number that will differenciate it from other global vars. Example valid labels: global004,global023,global047.
if the label is named !reset it will initialize all the states to off.
health: "global state"
this will store the state (on/off) of the entity. 0 means off && 1 means on. You will need something
like a trigger_condition to compare with the func_global.
---------------------------------------
flags:
none
---------------------------------------
*/

vector<int> dg_file_array(50);

void FuncGlobalWriteLineToFile(string filename);
void FuncGlobalReadFromFileToArray(string filename);

class CFuncGlobal : public CBaseEntity
{
public:
	float global_state;
	string_t mapname;
	string_t map_series;
	
	void Spawn()
	{
		//mapname = g_engfuncs.pfnCVarGetString("mapname");
		SetUse(&CFuncGlobal::TriggerUse);
		Use(NULL, NULL, USE_SET ,0.0); 			// load the values at map start 
	}
	
	void KeyValue(KeyValueData* pkvd) override
	{
		
		if(FStrEq(pkvd->szKeyName, "map_series"))
		{
			map_series = ALLOC_STRING(pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
		else 
		{
			return CBaseEntity::KeyValue(pkvd);
		}
	}
	
	void TriggerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
	{
		int a;
		int b;
		int c;
		int d;
		
		string global_name = STRING(pev->netname); 

		string dataglobal = string("scripts/maps/store/dataglobal_");
		
		if (!strcmp(STRING(pev->netname), "!reset")) // reset to 0 all global
		{
			for ( int I = 0; I < 50; I++ )
			{
				dg_file_array[I] = 0;
			}
			FuncGlobalWriteLineToFile(dataglobal + STRING(map_series) + ".txt");
			FuncGlobalReadFromFileToArray(dataglobal + STRING(map_series) + ".txt");
			return;
		}
		
		if (useType == USE_SET) // load state value
		{
			FuncGlobalReadFromFileToArray(dataglobal + STRING(map_series) + ".txt");
			int gi = atoi(global_name.substr(global_name.size()-3,global_name.size()-1).c_str());
			global_state = float(dg_file_array[gi]); 
			pev->health = global_state;
			return;
		}
		
		global_state = pev->health;
		
		if (useType == USE_OFF)
		{
			global_state = 0;
			pev->health = 0;
			int gi = atoi(global_name.substr(global_name.size()-3,global_name.size()-1).c_str());
			dg_file_array[gi] = int (global_state);
			FuncGlobalWriteLineToFile(dataglobal + STRING(map_series) + ".txt");
			FuncGlobalReadFromFileToArray(dataglobal + STRING(map_series) + ".txt");
			return;
		}
		
		if (useType == USE_ON)
		{
			global_state = 1;
			pev->health = 1;
			int gi = atoi(global_name.substr(global_name.size()-3,global_name.size()-1).c_str());
			dg_file_array[gi] = int (global_state);
			FuncGlobalWriteLineToFile(dataglobal + STRING(map_series) + ".txt");
			FuncGlobalReadFromFileToArray(dataglobal + STRING(map_series) + ".txt");
			return;
		}
		
		if (useType == USE_TOGGLE) // switch between off && on status then activate
		{
			FuncGlobalReadFromFileToArray(dataglobal + STRING(map_series) + ".txt");
			int gi = atoi(global_name.substr(global_name.size()-3,global_name.size()-1).c_str());
			ALERT( at_console, "%s", ("gi: "+to_string(gi)+"\n").c_str() );
			ALERT( at_console, "%s", ("sub: "+global_name.substr(global_name.size()-3,global_name.size()-1)+"\n").c_str() );
			global_state = float (dg_file_array[gi]);
			pev->health = global_state;
			
			if (global_state == 0)
			{
				global_state = 1;
				pev->health = 1;
				dg_file_array[gi] = int (global_state);
				FuncGlobalWriteLineToFile(dataglobal + STRING(map_series) + ".txt");
				FuncGlobalReadFromFileToArray(dataglobal + STRING(map_series) + ".txt");
				return;
			}
			else
			{
				global_state = 0;
				pev->health = 0;
				dg_file_array[gi] = int (global_state);
				FuncGlobalWriteLineToFile(dataglobal + STRING(map_series) + ".txt");
				FuncGlobalReadFromFileToArray(dataglobal + STRING(map_series) + ".txt");
				return;
			}
		}
		
		if (useType == USE_KILL) // this one does not work
		{
			UTIL_Remove( this );
			return;
		}
	}
};

void FuncGlobalWriteLineToFile(string filename)
{
	string StringToWrite;
	FILE* file = UTIL_OpenFile(filename.c_str(), "r");
	if(file)
	{
		char line[1024];
		while(fgets(line, sizeof(line), file))
		{
			if(line[0] == '#' || strlen(line) == 0)
				continue;
			StringToWrite = StringToWrite+ line +"\n";
		}
		fclose(file);
	}
	
	file = UTIL_OpenFile(filename.c_str(), "w");
	if(file) //delete the file && write its values again
	{
		for ( int I = 0; I < 50; I++ )
		{
			string prefix;
			if (I < 10) {prefix = "global00";} else {prefix = "global0";}
			
			StringToWrite = to_string(dg_file_array[I]);
			string line = prefix + to_string(I) + "=" + StringToWrite + "\n";
			fwrite(line.c_str(), line.size(), 1, file);;
		}
		fclose(file);
	}
}

void FuncGlobalReadFromFileToArray(string filename)
{
	FILE* file = UTIL_OpenFile(filename.c_str(), "r");
	int i = 0;
	if(file)
	{
		char line[1024];
		while (fgets(line, sizeof(line), file))
		{
			//fix for linux
			string sLine = line;
			string sFix = sLine.substr(sLine.size()-1,1);
			if(sFix == " " || sFix == "\n" || sFix == "\r" || sFix == "\t")
				sLine = sLine.substr(0, sLine.size()-1);
			if(sLine.substr(0,1) == "#" || sLine.empty())
			{
				continue;
			}
						
			dg_file_array[i] = atoi(sLine.substr(10, 1).c_str());
			
			i++;
		}
		fclose(file);
	}
}

LINK_ENTITY_TO_CLASS(func_global, CFuncGlobal)