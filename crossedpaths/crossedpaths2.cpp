// *todo Add credits.
// *todo use hooks

#include "asscart"
#include "laseractivation"
#include "messages"


void MapActivate()
{
        CBaseEntity* gt = UTIL_FindEntityByTargetname(NULL, 'gt_filesmissing');
	UTIL_Remove(gt);
}


/**
 * Massn train.
 */


void InitCart(CBaseEntity*, CBaseEntity*, USE_TYPE, float)
{
    *cart = Cart("ac");
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

void WaitForTrainToStopB(CBaseEntity* caller)
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


const string EN_TS_NEXT_WAVE = "_tr_tnl_nxtwave";

void NoMoreTnlWaveMassns(CBaseEntity* pActivator) {
    CBaseEntity* survivor = UTIL_FindEntityByTargetname(NULL, "mma_tnlwave");
    CBaseEntity* tnkController = UTIL_FindEntityByTargetname(NULL, "mma_tnltankctrler");
    if (survivor  == NULL  && tnkController  == NULL  ) {
        FireTargets(EN_TS_NEXT_WAVE, NULL, NULL, USE_ON);
    }
}


/**
 * Reloadable mounted gun. Makes it possible to insert in the map a controllable mounted gun
 * that needs to be reloaded with ammo boxes.
 *
 * *todo More modular.
 */


const string EN_MM_M2_TO_OFF = "_mm_tm2_off";
const string EN_MM_M2_TO_ON = "_mm_tm2_on";
const string EN_MM_AMMO_BOX_HOLO = "_mm_tm2_holo";
const string EN_FB_AMMO_BOX = "_fb_tm2_ammo";
const string EN_FI_RELOAD_ZONE = "_fi_tm2_rld";

int16 m2AmmoLeft = 15;

void OnShotFired(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float tmp) {
    m2AmmoLeft--;
    string txtMsg;
    
    if (m2AmmoLeft <= 0) {
        m2AmmoLeft = 0; // In case it accidently goes under 0.
        FireTargets(EN_MM_M2_TO_OFF, NULL, NULL, USE_OFF);
    } else if (m2AmmoLeft < 50) {
        snprintf(txtMsg, "Only %1 rounds left!", m2AmmoLeft);
        g_PlayerFuncs.HudMessageAll(CreateHudTextParams(WARNING_MSG, 4), txtMsg);
    } else {
        snprintf(txtMsg, "%1 rounds left", m2AmmoLeft);
        g_PlayerFuncs.HudMessageAll(CreateHudTextParams(INFO_MSG, 4), txtMsg);
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
        if (ammoBox  && ammoBox.Intersects(ammoZone)) {
            isZoneEmpty = false;

            if (0 == m2AmmoLeft) {
                FireTargets(EN_MM_M2_TO_ON, NULL, NULL, USE_OFF);
            }

            m2AmmoLeft += uint16(cast<CBaseDelay*>(ammoBox).m_flDelay);
            UTIL_Remove(ammoBox);
            string txtMsg;
            snprintf(txtMsg, "Mounted gun reloaded. %1 rounds left.", m2AmmoLeft);
            g_PlayerFuncs.HudMessageAll(CreateHudTextParams(SUCCESS_MSG, 4), txtMsg);
        }
    } while (currentBoxHandle);
    if (isZoneEmpty) {
            FireTargets(EN_MM_AMMO_BOX_HOLO, NULL, NULL, USE_OFF);
    }
}
