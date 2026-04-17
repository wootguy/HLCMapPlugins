#include "extdll.h"
#include "util.h"
#include "crossedpaths.h"


TowerOfHanoi::TowerOfHanoi(
    const char* id,
    const int diskHeight, 
    const int nDisks,
    const char* successEntityName
) {
    m_id = id;

    m_successEntityName = successEntityName;
    m_diskHeight = diskHeight;
    m_nDisks = nDisks;
    m_zReference = UTIL_FindEntityByTargetname(NULL, UTIL_VarArgs("pc_%s_0", id))->pev->origin.z;
}

void TowerOfHanoi::Move(const int sourceId, const int targetId) {
    // UTIL_ClientPrintAll(HUD_PRINTCONSOLE, "Move method called.");
    FireTargets(UTIL_VarArgs("tr_%s_ms", m_id), NULL, NULL, USE_OFF);

    std::vector<CBaseEntity*> sourceDisks = getDisks(sourceId);

    if (sourceDisks.size() == 0) {
        // UTIL_ClientPrintAll(HUD_PRINTCONSOLE, "No disks.");
        FireTargets(UTIL_VarArgs("tr_%s_ms", m_id), NULL, NULL, USE_OFF);
        return;
    }

    CBaseEntity* disk = sourceDisks[sourceDisks.size() - 1];

    std::vector<CBaseEntity*>  targetDisks = getDisks(targetId);

    if (targetDisks.size() > 0 && GetDiskSize(disk) > GetDiskSize(targetDisks[targetDisks.size() - 1])) {
        FireTargets(UTIL_VarArgs("tr_%s_ms", m_id), NULL, NULL, USE_OFF);
        FireTargets(UTIL_VarArgs("gt_%s_disktoobig", m_id), NULL, NULL, USE_OFF);
        return;
    }

    CBaseEntity* targetPc = UTIL_FindEntityByTargetname(NULL, UTIL_VarArgs("pc_%s_%d", m_id, targetId));
    Vector newOrigin = targetPc->pev->origin;
    newOrigin.z = m_zReference + m_diskHeight * targetDisks.size();
    UTIL_SetOrigin(targetPc->pev, newOrigin);
    CBaseEntity* pc = UTIL_FindEntityByTargetname(NULL, UTIL_VarArgs("pc_%s", + m_id));

    pc->pev->target = targetPc->pev->targetname;
    // If this move ends the puzzle.
    if (2 == targetId && targetDisks.size() + 1 == m_nDisks) {
        targetPc->pev->message = ALLOC_STRING(m_successEntityName);
    }
    // UTIL_ClientPrintAll(HUD_PRINTCONSOLE, "Moving disk.");
    disk->Use(NULL, NULL, USE_ON);
}

std::vector<CBaseEntity*> TowerOfHanoi::getDisks(const int stackId) {
    CBaseEntity* stackPc = UTIL_FindEntityByTargetname(NULL, UTIL_VarArgs("pc_%s_%d", m_id, stackId));
    Vector stackOrigin = stackPc->pev->origin;
    stackOrigin.z = m_zReference;
    Vector stackTopOrigin = stackOrigin + Vector(0, 0, m_diskHeight * m_nDisks);
    std::vector<CBaseEntity*> disks(N_ENTITIES_BUFFER);
    memset(&disks[0], 0, sizeof(disks));
    int nBrushEntities = UTIL_BrushEntitiesInBox(&disks[0], disks.size(), stackOrigin, stackTopOrigin);
    disks.resize(nBrushEntities);

    int n = 0;
    while (n < disks.size() && !disks.empty() && NULL != disks[n]) {
        if ((std::string("ft_") + m_id) != STRING(disks[n]->pev->targetname)) {
            disks.erase(disks.begin() + n);
        } else {
            n++;
        }
    }

    for (int i = 0; i < disks.size(); i++) {
        for (int j = i + 1; j < disks.size(); j++) {
            CBaseEntity* currentDisk = disks[j];
            if (currentDisk == NULL) {
                continue;
            } else if (disks[i] == NULL) {
                disks[j] = NULL;
                disks[i] = currentDisk;
            } else if (GetDiskSize(disks[i]) < GetDiskSize(currentDisk)) {
                disks[j] = disks[i];
                disks[i] = currentDisk;
            }
        }
    }

    return disks;
}


int TowerOfHanoi::GetDiskSize(CBaseEntity* disk) {
    return disk->GetCustomKeyValue("$i_size").iVal;
}