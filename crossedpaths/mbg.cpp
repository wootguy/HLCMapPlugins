const string EN_MBG = "_mbg_bgrm";
const string EN_MBG_DEAD = "_mm_end";
const string EN_GT_MBG_LEFT = "_gt_end_mbgleft";

/**
 * Make sure there are no bodyguards left.
 */
void FireIfMbgAreDead(CBaseEntity* pActivator, CBaseEntity* caller, USE_TYPE useType, float flValue) {
    CBaseEntity* bg = UTIL_FindEntityByTargetname(NULL, EN_MBG);
    if (bg  == NULL ) {
        FireTargets(EN_MBG_DEAD, NULL, NULL, USE_ON);
    } else {
        FireTargets(EN_GT_MBG_LEFT, NULL, NULL, USE_ON);
    }
}
