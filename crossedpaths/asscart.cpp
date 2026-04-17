#include "extdll.h"
#include "util.h"
#include "CBaseEntity.h"
#include "crossedpaths.h"

Cart::Cart(const char* sCartId) {
    m_sCartId = sCartId;

    m_ftDr0 = UTIL_FindEntityByTargetname(NULL, EN_FT_STATIC_DR_0);
    m_ftDr1 = UTIL_FindEntityByTargetname(NULL, EN_FT_STATIC_DR_1);
    m_fttDynamicCart = UTIL_FindEntityByTargetname(NULL, EN_FTT_DYNAMIC_CART);
    m_fwStaticStructure = UTIL_FindEntityByTargetname(NULL, EN_FW_STATIC_STRUCTURE);
    m_pcDr0_0 = UTIL_FindEntityByTargetname(NULL, EN_PC_STATIC_DR_0_0);
    m_pcDr0_1 = UTIL_FindEntityByTargetname(NULL, EN_PC_STATIC_DR_0_1);
    m_pcDr1_0 = UTIL_FindEntityByTargetname(NULL, EN_PC_STATIC_DR_1_0);
    m_pcDr1_1 = UTIL_FindEntityByTargetname(NULL, EN_PC_STATIC_DR_1_1);
    CBaseEntity* storage = UTIL_FindEntityByTargetname(NULL, EN_IT_STORAGE);

    if (!m_ftDr0 || !m_ftDr1 || !m_fttDynamicCart || !m_fwStaticStructure || !m_pcDr0_0
        || !m_pcDr0_1 || !m_pcDr1_0 || !m_pcDr1_1 || !storage) {
        return;
    }

    m_vRelOriginFtDr0 = m_ftDr0->pev->origin - m_fwStaticStructure->pev->origin;
    m_vRelOriginPcDr0_0 = m_pcDr0_0->pev->origin - m_fwStaticStructure->pev->origin;
    m_vRelOriginPcDr0_1 = m_pcDr0_1->pev->origin - m_fwStaticStructure->pev->origin;
    m_vRelOriginFtDr1 = m_ftDr1->pev->origin - m_fwStaticStructure->pev->origin;
    m_vRelOriginPcDr1_0 = m_pcDr1_0->pev->origin - m_fwStaticStructure->pev->origin;
    m_vRelOriginPcDr1_1 = m_pcDr1_1->pev->origin - m_fwStaticStructure->pev->origin;
    m_vStorage = storage->pev->origin;
}

void Cart::SwapStaticForDyn() {
    if (m_fwStaticStructure)
        UTIL_SetOrigin(m_fttDynamicCart->pev, m_fwStaticStructure->pev->origin);

    UTIL_SetOrigin(m_ftDr0->pev, m_vStorage + m_vRelOriginFtDr0);
    UTIL_SetOrigin(m_ftDr1->pev, m_vStorage + m_vRelOriginFtDr1);
    UTIL_SetOrigin(m_pcDr0_0->pev, m_vStorage + m_vRelOriginPcDr0_0);
    UTIL_SetOrigin(m_pcDr0_1->pev, m_vStorage + m_vRelOriginPcDr0_1);
    UTIL_SetOrigin(m_pcDr1_0->pev, m_vStorage + m_vRelOriginPcDr1_0);
    UTIL_SetOrigin(m_pcDr1_1->pev, m_vStorage + m_vRelOriginPcDr1_1);
    UTIL_SetOrigin(m_fwStaticStructure->pev, m_vStorage);
}

void Cart::SwapDynForStatic() {
    // UTIL_ClientPrintAll(HUD_PRINTCONSOLE, m_fttDynamicCart->pev->angles.ToString());

    Vector rotatedRelOriginFtDr0 = RotatePosition(m_vRelOriginFtDr0, ToRad(m_fttDynamicCart->pev->angles.y));
    UTIL_SetOrigin(m_ftDr0->pev, m_fttDynamicCart->pev->origin + rotatedRelOriginFtDr0);
    m_ftDr0->pev->angles = m_fttDynamicCart->pev->angles;

    Vector rotatedRelOriginPcDr0_0 = RotatePosition(m_vRelOriginPcDr0_0, ToRad(m_fttDynamicCart->pev->angles.y));
    UTIL_SetOrigin(m_pcDr0_0->pev, m_fttDynamicCart->pev->origin + rotatedRelOriginPcDr0_0);

    Vector rotatedRelOriginPcDr0_1 = RotatePosition(m_vRelOriginPcDr0_1, ToRad(m_fttDynamicCart->pev->angles.y));
    UTIL_SetOrigin(m_pcDr0_1->pev, m_fttDynamicCart->pev->origin + rotatedRelOriginPcDr0_1);

    Vector rotatedRelOriginFtDr1 = RotatePosition(m_vRelOriginFtDr1, ToRad(m_fttDynamicCart->pev->angles.y));
    UTIL_SetOrigin(m_ftDr1->pev, m_fttDynamicCart->pev->origin + rotatedRelOriginFtDr1);
    m_ftDr1->pev->angles = m_fttDynamicCart->pev->angles;

    Vector rotatedRelOriginPcDr1_0 = RotatePosition(m_vRelOriginPcDr1_0, ToRad(m_fttDynamicCart->pev->angles.y));
    UTIL_SetOrigin(m_pcDr1_0->pev, m_fttDynamicCart->pev->origin + rotatedRelOriginPcDr1_0);

    Vector rotatedRelOriginPcDr1_1 = RotatePosition(m_vRelOriginPcDr1_1, ToRad(m_fttDynamicCart->pev->angles.y));
    UTIL_SetOrigin(m_pcDr1_1->pev, m_fttDynamicCart->pev->origin + rotatedRelOriginPcDr1_1);

    UTIL_SetOrigin(m_fwStaticStructure->pev, m_fttDynamicCart->pev->origin);
    m_fwStaticStructure->pev->angles = m_fttDynamicCart->pev->angles;

    UTIL_SetOrigin(m_fttDynamicCart->pev, m_vStorage);
}

void Cart::WaitForTrainToStop(const char* target)
{
    // UTIL_ClientPrintAll(HUD_PRINTCONSOLE, string(massnCart->pev->speed) + "\n");
    if (m_fttDynamicCart->pev->speed == 0) {
        FireTargets(target, m_fttDynamicCart, m_fttDynamicCart, USE_ON);
    }
}

// *todo Make private
double Cart::ToRad(const double degree) {
    return degree * S_PI / 180;
}

// *todo Make private
Vector Cart::RotatePosition(Vector initialPosition, const double angle_rad) {
    return Vector(
        initialPosition.x * cos(angle_rad) - initialPosition.y * (-sin(angle_rad)),
        initialPosition.x * sin(angle_rad) + initialPosition.y * cos(angle_rad),
        initialPosition.z
    );
}

/// *todo Optimise.
/// *todo Rename.
/// *todo Hard-coded values (entity names, minimum distance).
/// *todo Use tracing.
// void ManageCartCrash(CBaseEntity *pActivator)
// {
//     CBaseEntity* ftt_train = UTIL_FindEntityByTargetname(NULL, "ftt_train");
//     CBaseEntity* ftt_massncart = UTIL_FindEntityByTargetname(NULL, "ftt_cart_0");
//     Vector sub =  ftt_massncart.GetOrigin().opSub(ftt_train.GetOrigin());
//     if (abs(sub.x) < 90 && abs(sub.y) < 90 && abs(sub.z) < 90) {
//         FireTargets("mm_ftttraincrash", ftt_massncart, ftt_massncart, USE_ON);
//     }
// }
