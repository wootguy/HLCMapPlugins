/**
 * *todo Magic strings.
 */
#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "crossedpaths.h"

LaserActivation::LaserActivation(CBaseEntity* activator, const char* target) {
    m_activator = activator;
    m_target = target;
}

void LaserActivation::React() {
    bool areTurretsAllies = AreTurretsAllies();
    CBaseMonster* m = m_activator->MyMonsterPointer();
    bool isPlayerActivator = m_activator->IsPlayer() || m->IsPlayerAlly();
    if (areTurretsAllies && !isPlayerActivator) {
        FireTargets(m_target, m_activator, m_activator, USE_ON);
    }
    else if (!areTurretsAllies && isPlayerActivator) {
        FireTargets(m_target, m_activator, m_activator, USE_ON);
        if (m_activator->IsPlayer()) {
            NameShame();
        }
    }
}

// *todo make private
bool LaserActivation::AreTurretsAllies() {
    const char* nextTarget = STRING(UTIL_FindEntityByTargetname(NULL, "tr_switchtrt")
        ->GetNextTarget()
        ->pev->targetname);

    return strcmp(nextTarget, "mm_trtally");
}

// *todo make private
void LaserActivation::NameShame() {
    CBasePlayer* player = m_activator->MyPlayerPointer();
    const char* txtMessage = UTIL_VarArgs("%s just tripped on a laser && activated the turrets! Well done...",
        STRING(player->pev->netname));
    UTIL_HudMessageAll(CreateHudTextParams(WARNING_MSG, 1), txtMessage);
}
