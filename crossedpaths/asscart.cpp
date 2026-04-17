Cart* cart;

const double S_PI = 3.141592653589793;
const string EN_FT_STATIC_DR_0 = "ft_ac_dr_0";
const string EN_FT_STATIC_DR_1 = "ft_ac_dr_1";
const string EN_FTT_DYNAMIC_CART = "ftt_ac";
const string EN_FW_STATIC_STRUCTURE = "fw_ac_struc";
const string EN_IT_STORAGE = "it_tankstorage";
const string EN_MM_ON_STOP_IN_B = "mm_ac_onstop_b";
const string EN_PC_STATIC_DR_0_0 = "pc_ac_dr_0_0";
const string EN_PC_STATIC_DR_0_1 = "pc_ac_dr_0_1";
const string EN_PC_STATIC_DR_1_0 = "pc_ac_dr_1_0";
const string EN_PC_STATIC_DR_1_1 = "pc_ac_dr_1_1";


final class Cart
{
    private CBaseEntity* m_ftDr0;
    private CBaseEntity* m_ftDr1;
    private CBaseEntity* m_fttDynamicCart;
    private CBaseEntity* m_fwStaticStructure;
    private CBaseEntity* m_pcDr0_0;
    private CBaseEntity* m_pcDr0_1;
    private CBaseEntity* m_pcDr1_0;
    private CBaseEntity* m_pcDr1_1;
    private string m_sCartId;
    private Vector m_vRelOriginFtDr0;
    private Vector m_vRelOriginFtDr1;
    private Vector m_vRelOriginPcDr0_0;
    private Vector m_vRelOriginPcDr0_1;
    private Vector m_vRelOriginPcDr1_0;
    private Vector m_vRelOriginPcDr1_1;
    private Vector m_vStorage;

    Cart(const string&sCartId) {
        m_sCartId = sCartId;

        *m_ftDr0 = UTIL_FindEntityByTargetname(NULL, EN_FT_STATIC_DR_0);
        *m_ftDr1 = UTIL_FindEntityByTargetname(NULL, EN_FT_STATIC_DR_1);
        *m_fttDynamicCart = UTIL_FindEntityByTargetname(NULL, EN_FTT_DYNAMIC_CART);
        *m_fwStaticStructure = UTIL_FindEntityByTargetname(NULL, EN_FW_STATIC_STRUCTURE);
        *m_pcDr0_0 = UTIL_FindEntityByTargetname(NULL, EN_PC_STATIC_DR_0_0);
        *m_pcDr0_1 = UTIL_FindEntityByTargetname(NULL, EN_PC_STATIC_DR_0_1);
        *m_pcDr1_0 = UTIL_FindEntityByTargetname(NULL, EN_PC_STATIC_DR_1_0);
        *m_pcDr1_1 = UTIL_FindEntityByTargetname(NULL, EN_PC_STATIC_DR_1_1);

        m_vRelOriginFtDr0 = m_ftDr0.GetOrigin().opSub(m_fwStaticStructure.GetOrigin());
        m_vRelOriginPcDr0_0 = m_pcDr0_0.GetOrigin().opSub(m_fwStaticStructure.GetOrigin());
        m_vRelOriginPcDr0_1 = m_pcDr0_1.GetOrigin().opSub(m_fwStaticStructure.GetOrigin());
        m_vRelOriginFtDr1 = m_ftDr1.GetOrigin().opSub(m_fwStaticStructure.GetOrigin());
        m_vRelOriginPcDr1_0 = m_pcDr1_0.GetOrigin().opSub(m_fwStaticStructure.GetOrigin());
        m_vRelOriginPcDr1_1 = m_pcDr1_1.GetOrigin().opSub(m_fwStaticStructure.GetOrigin());
        m_vStorage = UTIL_FindEntityByTargetname(NULL, EN_IT_STORAGE).GetOrigin();
    }

    void SwapStaticForDyn() {
        m_fttDynamicCart.SetOrigin(m_fwStaticStructure.GetOrigin());

        m_ftDr0.SetOrigin(m_vStorage.opAdd(m_vRelOriginFtDr0));
        m_ftDr1.SetOrigin(m_vStorage.opAdd(m_vRelOriginFtDr1));
        m_pcDr0_0.SetOrigin(m_vStorage.opAdd(m_vRelOriginPcDr0_0));
        m_pcDr0_1.SetOrigin(m_vStorage.opAdd(m_vRelOriginPcDr0_1));
        m_pcDr1_0.SetOrigin(m_vStorage.opAdd(m_vRelOriginPcDr1_0));
        m_pcDr1_1.SetOrigin(m_vStorage.opAdd(m_vRelOriginPcDr1_1));
        m_fwStaticStructure.SetOrigin(m_vStorage);
    }

    void SwapDynForStatic() {
        // UTIL_ClientPrintAll(HUD_PRINTCONSOLE, m_fttDynamicCart->pev->angles.ToString());

        Vector rotatedRelOriginFtDr0 = RotatePosition(m_vRelOriginFtDr0, ToRad(m_fttDynamicCart->pev->angles.y));
        m_ftDr0.SetOrigin(m_fttDynamicCart.GetOrigin().opAdd(rotatedRelOriginFtDr0));
        m_ftDr0->pev->angles = m_fttDynamicCart->pev->angles;

        Vector rotatedRelOriginPcDr0_0 = RotatePosition(m_vRelOriginPcDr0_0, ToRad(m_fttDynamicCart->pev->angles.y));
        m_pcDr0_0.SetOrigin(m_fttDynamicCart.GetOrigin().opAdd(rotatedRelOriginPcDr0_0));

        Vector rotatedRelOriginPcDr0_1 = RotatePosition(m_vRelOriginPcDr0_1, ToRad(m_fttDynamicCart->pev->angles.y));
        m_pcDr0_1.SetOrigin(m_fttDynamicCart.GetOrigin().opAdd(rotatedRelOriginPcDr0_1));

        Vector rotatedRelOriginFtDr1 = RotatePosition(m_vRelOriginFtDr1, ToRad(m_fttDynamicCart->pev->angles.y));
        m_ftDr1.SetOrigin(m_fttDynamicCart.GetOrigin().opAdd(rotatedRelOriginFtDr1));
        m_ftDr1->pev->angles = m_fttDynamicCart->pev->angles;

        Vector rotatedRelOriginPcDr1_0 = RotatePosition(m_vRelOriginPcDr1_0, ToRad(m_fttDynamicCart->pev->angles.y));
        m_pcDr1_0.SetOrigin(m_fttDynamicCart.GetOrigin().opAdd(rotatedRelOriginPcDr1_0));

        Vector rotatedRelOriginPcDr1_1 = RotatePosition(m_vRelOriginPcDr1_1, ToRad(m_fttDynamicCart->pev->angles.y));
        m_pcDr1_1.SetOrigin(m_fttDynamicCart.GetOrigin().opAdd(rotatedRelOriginPcDr1_1));

        m_fwStaticStructure.SetOrigin(m_fttDynamicCart.GetOrigin());
        m_fwStaticStructure->pev->angles = m_fttDynamicCart->pev->angles;

        m_fttDynamicCart.SetOrigin(m_vStorage);
    }

    void WaitForTrainToStop(const string &in target)
    {
        // UTIL_ClientPrintAll(HUD_PRINTCONSOLE, string(massnCart->pev->speed) + "\n");
        if (m_fttDynamicCart->pev->speed == 0) {
            FireTargets(target, m_fttDynamicCart, m_fttDynamicCart, USE_ON);
        }
    }

    // *todo Make private
    double ToRad(const double &in degree) {
        return degree * S_PI / 180;
    }

    // *todo Make private
    Vector RotatePosition(Vector initialPosition, const double angle_rad) {
        return Vector(
            initialPosition.x * cos(angle_rad) - initialPosition.y * (-sin(angle_rad)),
            initialPosition.x * sin(angle_rad) + initialPosition.y * cos(angle_rad),
            initialPosition.z
        );
    }
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
