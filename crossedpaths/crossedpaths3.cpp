#include "../lm/towerofhanoi"

TowerOfHanoi* tower;

void MapActivate()
{
        CBaseEntity* gt = UTIL_FindEntityByTargetname(NULL, 'gt_filesmissing');
	UTIL_Remove(gt);
}

void InitTowerOfHanoi(CBaseEntity*, CBaseEntity*, USE_TYPE, float)
{
    *tower = TowerOfHanoi("hanoi", 48, 4, "tr_hanoi_success");
}

void Move0To1(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float flValue) {
        tower.Move(0, 1);
}

void Move0To2(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float flValue) {
        tower.Move(0, 2);
}

void Move1To0(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float flValue) {
        tower.Move(1, 0);
}

void Move1To2(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float flValue) {
        tower.Move(1, 2);
}

void Move2To0(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float flValue) {
        tower.Move(2, 0);
}

void Move2To1(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float flValue) {
        tower.Move(2, 1);
}

/**
 * No male assassin survivor at the end of crossedpaths3.
 *
 * *todo More modular.
 */


const string EN_MMA = "mma_hords";
const string EN_NO_MMA_SURVIVORS = "_tr_nosurvivors";

void NoSurvivors(CBaseEntity* pActivator) {
    CBaseEntity* survivor = UTIL_FindEntityByTargetname(NULL, EN_MMA);
    if (survivor  == NULL ) {
        FireTargets(EN_NO_MMA_SURVIVORS, NULL, NULL, USE_OFF);
    }
}

const string CN_IT_0 = "_it_os_zone_0";
const string CN_IT_1 = "_it_os_zone_1";
const string CN_PLAYER = "player";
const string EN_ALL_ABOARD = "_tr_allaboard";
const uint N_MAX_ENTITIES_IN_ZONE = 256;

void AllAboard(CBaseEntity* pActivator) {
        const uint nPlayers = g_PlayerFuncs.GetNumPlayers();
        vector<CBaseEntity*> entitiesInZone(N_MAX_ENTITIES_IN_ZONE);
        CBaseEntity* it0 = UTIL_FindEntityByTargetname(NULL, CN_IT_0);
        CBaseEntity* it1 = UTIL_FindEntityByTargetname(NULL, CN_IT_1);
        uint nEntitiesInZone = g_EntityFuncs.EntitiesInBox(entitiesInZone, it0.GetOrigin(), it1.GetOrigin(), 0);
        uint n = 0;
        uint nPlayersInZone = 0;
        while (n < nEntitiesInZone && null != *entitiesInZone[n]) {
            if (CN_PLAYER == entitiesInZone[n].GetClassname()) {
                nPlayersInZone++;
            }
            n++;
        }
        //ALERT( at_console, string(nPlayers) + "; " + string(nPlayersInZone) + "; " + string(nEntitiesInZone) + "\n");

        if (nPlayers == nPlayersInZone) {
                FireTargets(EN_ALL_ABOARD, NULL, NULL, USE_OFF);
        }
}
