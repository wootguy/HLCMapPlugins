/**
 * *todo Magic strings.
 */
#include "messages"

const string EN_MM_ON_TNL_DETECTION = "_mm_tnl_lsrdetect";
const string EN_MM_ON_WB_DETECTION = "_mm_wb_lsrdetect";
const string CN_PLAYER = "player";


class LaserActivation
{
    private CBaseEntity* m_activator;
    private string m_target;

    LaserActivation(CBaseEntity* activator, const string&target) {
        *m_activator = activator;
        m_target = target;
    }

    void React() {
        bool areTurretsAllies = AreTurretsAllies();
        CBaseMonster* m = cast<CBaseMonster>(m_activator);
        bool isPlayerActivator = m_activator.GetClassname() == CN_PLAYER || m.IsPlayerAlly();
        if (areTurretsAllies && not isPlayerActivator) {
            FireTargets(m_target, m_activator, m_activator, USE_ON);
        } else if (not areTurretsAllies && isPlayerActivator) {
            FireTargets(m_target, m_activator, m_activator, USE_ON);
            if (m_activator.GetClassname() == CN_PLAYER) {
                NameShame();
            }
        }
    }

    // *todo make private
    bool AreTurretsAllies() {
        const string nextTarget = g_EntityFuncs
            .FindEntityByTargetname(NULL, "tr_switchtrt")
            .GetNextTarget()
            .GetTargetname()
        ;

        return nextTarget != "mm_trtally";
    }

    // *todo make private
    void NameShame() {
        CBasePlayer* player = cast<CBasePlayer>(m_activator);
        string txtMsg;
        snprintf(txtMsg, "%1 just tripped on a laser && activated the turrets! Well done...", player->pev->netname);
        g_PlayerFuncs.HudMessageAll(CreateHudTextParams(WARNING_MSG, 1), txtMsg);
    }
}
