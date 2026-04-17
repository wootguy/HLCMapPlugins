#include "extdll.h"
#include "util.h"
#include "crossedpaths.h"
#include "CBaseDelay.h"

// *todo Add credits.
// *todo use hooks

Cart cart;
TowerOfHanoi tower;

HOOK_RETURN_DATA MapActivate()
{
    CBaseEntity* gt = UTIL_FindEntityByTargetname(NULL, "gt_filesmissing");
	UTIL_Remove(gt);

    return HOOK_CONTINUE;
}


//
// crossedpaths2 funcs
//

/**
 * Massn train.
 */


void InitCart(CBaseEntity*, CBaseEntity*, USE_TYPE, float)
{
    cart = Cart("ac");
}

void SwapDynCartForStaticA(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float flValue)
{
    cart.SwapDynForStatic();
    FireTargets("tr_ac_afterstop_a", pActivator, caller, USE_ON);
}

void SwapDynCartForStaticB(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float flValue)
{
    cart.SwapDynForStatic();
    FireTargets("tr_ac_afterstop_b", pActivator, caller, USE_ON);
}

void WaitForTrainToStopB(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float flValue)
{
    cart.WaitForTrainToStop(EN_MM_ON_STOP_IN_B);
}

// *todo better name?
void SwapStaticCartForDyn(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float tmp) {
    cart.SwapStaticForDyn();
    FireTargets(EN_FTT_DYNAMIC_CART, pActivator, caller, USE_ON);
}


/**
 * Laser detection.
 */


void OnTnlLaserActivation(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float tmp)
{
    auto activation = LaserActivation(pActivator, EN_MM_ON_TNL_DETECTION);
    activation.React();
}

void OnWBLaserActivation(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float tmp)
{
    auto activation = LaserActivation(pActivator, EN_MM_ON_WB_DETECTION);
    activation.React();
}


/**
 * Waves.
 *
 * *todo More modular.
 */


const char* EN_TS_NEXT_WAVE = "_tr_tnl_nxtwave";

void NoMoreTnlWaveMassns(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float tmp) {
    CBaseEntity* survivor = NULL;
    while (survivor = UTIL_FindEntityByTargetname(survivor, "mma_tnlwave")) {
        if (survivor->IsAlive()) {
            return;
        }
    }

    CBaseEntity* tnkController = UTIL_FindEntityByTargetname(NULL, "mma_tnltankctrler");
    if ( tnkController  == NULL  ) {
        FireTargets(EN_TS_NEXT_WAVE, NULL, NULL, USE_ON);
    }
}


/**
 * Reloadable mounted gun. Makes it possible to insert in the map a controllable mounted gun
 * that needs to be reloaded with ammo boxes.
 *
 * *todo More modular.
 */


const char* EN_MM_M2_TO_OFF = "_mm_tm2_off";
const char* EN_MM_M2_TO_ON = "_mm_tm2_on";
const char* EN_MM_AMMO_BOX_HOLO = "_mm_tm2_holo";
const char* EN_FB_AMMO_BOX = "_fb_tm2_ammo";
const char* EN_FI_RELOAD_ZONE = "_fi_tm2_rld";

int m2AmmoLeft = 15;

void OnShotFired(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float tmp) {
    m2AmmoLeft--;
    
    if (m2AmmoLeft <= 0) {
        m2AmmoLeft = 0; // In case it accidently goes under 0.
        FireTargets(EN_MM_M2_TO_OFF, NULL, NULL, USE_OFF);
    } else if (m2AmmoLeft < 50) {
        UTIL_HudMessageAll(CreateHudTextParams(WARNING_MSG, 4), UTIL_VarArgs(
            "Only %d rounds left!", m2AmmoLeft));
    } else {
        UTIL_HudMessageAll(CreateHudTextParams(INFO_MSG, 4), UTIL_VarArgs(
            "%d rounds left", m2AmmoLeft));
    }
}

void ReloadMountedGun(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float tmp) {
    CBaseEntity* ammoZone = UTIL_FindEntityByTargetname(NULL, EN_FI_RELOAD_ZONE);
    EHANDLE currentBoxHandle = NULL;
    bool isZoneEmpty = true;

    do {
        CBaseEntity* ammoBox = UTIL_FindEntityByTargetname(currentBoxHandle, EN_FB_AMMO_BOX);
        // if (ammoBox ) {
        //     UTIL_ClientPrintAll(HUD_PRINTCONSOLE, ammoBox.GetTargetname());
        // }
        currentBoxHandle = ammoBox;
        if (ammoBox && ammoBox->Intersects(ammoZone)) {
            isZoneEmpty = false;

            if (0 == m2AmmoLeft) {
                FireTargets(EN_MM_M2_TO_ON, NULL, NULL, USE_OFF);
            }

            m2AmmoLeft += ammoBox->MyDelayPointer()->m_flDelay;
            UTIL_Remove(ammoBox);
            UTIL_HudMessageAll(CreateHudTextParams(SUCCESS_MSG, 4), UTIL_VarArgs(
                "Mounted gun reloaded. %d rounds left.", m2AmmoLeft));
            break;
        }
    } while (currentBoxHandle);

    if (isZoneEmpty) {
            FireTargets(EN_MM_AMMO_BOX_HOLO, NULL, NULL, USE_OFF);
    }
}



//
// crossedpaths3 funcs
//

void InitTowerOfHanoi(CBaseEntity*, CBaseEntity*, USE_TYPE, float)
{
    tower = TowerOfHanoi("hanoi", 48, 4, "tr_hanoi_success");
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


const char* EN_MMA = "mma_hords";
const char* EN_NO_MMA_SURVIVORS = "_tr_nosurvivors";

void NoSurvivors(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float flValue) {
    CBaseEntity* survivor = UTIL_FindEntityByTargetname(NULL, EN_MMA);
    if (survivor == NULL) {
        FireTargets(EN_NO_MMA_SURVIVORS, NULL, NULL, USE_OFF);
    }
}

const char* CN_IT_0 = "_it_os_zone_0";
const char* CN_IT_1 = "_it_os_zone_1";
const char* CN_PLAYER = "player";
const char* EN_ALL_ABOARD = "_tr_allaboard";
const int N_MAX_ENTITIES_IN_ZONE = 256;

void AllAboard(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float flValue) {
    const int nPlayers = UTIL_CountPlayers();
    CBaseEntity* entitiesInZone[N_MAX_ENTITIES_IN_ZONE];
    memset(entitiesInZone, 0, sizeof(entitiesInZone));

    CBaseEntity* it0 = UTIL_FindEntityByTargetname(NULL, CN_IT_0);
    CBaseEntity* it1 = UTIL_FindEntityByTargetname(NULL, CN_IT_1);
    int nEntitiesInZone = UTIL_EntitiesInBox(entitiesInZone, N_MAX_ENTITIES_IN_ZONE, it0->pev->origin, it1->pev->origin, FL_CLIENT, true);
    int n = 0;
    int nPlayersInZone = 0;
    while (n < nEntitiesInZone && NULL != entitiesInZone[n]) {
        nPlayersInZone++;
        n++;
    }
    //ALERT( at_console, string(nPlayers) + "; " + string(nPlayersInZone) + "; " + string(nEntitiesInZone) + "\n");

    if (nPlayers == nPlayersInZone) {
        FireTargets(EN_ALL_ABOARD, NULL, NULL, USE_OFF);
    }
}


HLCOOP_PLUGIN_HOOKS g_hooks;

extern "C" int DLLEXPORT PluginInit() {
    g_hooks.pfnMapStart = MapActivate;

    RegisterPluginEntCallback(InitCart);
    RegisterPluginEntCallback(SwapDynCartForStaticA);
    RegisterPluginEntCallback(SwapDynCartForStaticB);
    RegisterPluginEntCallback(WaitForTrainToStopB);
    RegisterPluginEntCallback(SwapStaticCartForDyn);
    RegisterPluginEntCallback(OnTnlLaserActivation);
    RegisterPluginEntCallback(OnWBLaserActivation);
    RegisterPluginEntCallback(OnShotFired);
    RegisterPluginEntCallback(ReloadMountedGun);
    RegisterPluginEntCallback(NoMoreTnlWaveMassns);

    RegisterPluginEntCallback(InitTowerOfHanoi);
    RegisterPluginEntCallback(Move0To1);
    RegisterPluginEntCallback(Move0To2);
    RegisterPluginEntCallback(Move1To0);
    RegisterPluginEntCallback(Move1To2);
    RegisterPluginEntCallback(Move2To0);
    RegisterPluginEntCallback(Move2To1);
    RegisterPluginEntCallback(AllAboard);
    RegisterPluginEntCallback(NoSurvivors);

    return RegisterPlugin(&g_hooks);
}

extern "C" void DLLEXPORT PluginExit() {
    // nothing to clean up
}