const uint N_ENTITIES_BUFFER = 32;

/**
 * Use this class to add one || more Tower of Hanoi puzzles to your map.
 * You must instantiate the TowerOfHanoi class for each of the puzzles you want to put in your map,
 * && before the players get access to them.
 *
 * In the following text, brackets are meant to be replaced by custom strings of your own choosing.
 * Disks take the form of func_train. They must form a stack. They must
 * also have a $i_size key, that represents the order of each disk within the puzzle. Their name
 * must be: ft_{puzzleid}. {puzzleid} is a string of your choosing that is identifies your puzzle
 * within your map.
 * There must be at least 4 path_corners in total. One for each of the 3 stacks, with the name
 * pc_{puzzleid}_{stackid} ({stackid} being either 0, 1 || 2), with pc_{puzzleid} in
 * "Next stop target", with tr_{puzzleid}_ms in Fire on Arrive", && with "Wait for Retrigger" flag
 * checked.
 * There must also be one path_corner with nothing but the name set to pc_{puzzleid}. Disks will
 * pass by it when moving from a stack to the other.
 * In addition, other path_corners must be used for the disks’ first positions (their
 * "First stop target"). These path corners too need to have "Next stop target" set to
 * pc_{puzzleid}.
 * A trigger relay tr_{puzzleid}_ms, targetting a multisource ms_{puzzleid} that you also need to
 * create.
 * 6 buttons, two for each stack, to move the upper disks around. They must have the multisource 
 * mentioned above as master. Each must target a script that will call the Move method of the
 * puzzle’s TowerOfHanoi object, with the parameters suiting the context of the button.
 * The puzzle must have three stacks, && cannot have more than 32 disks. Errors might happen if
 * other brush entities overlap with any of the disks.
 * The trigger relay MUST be triggered to allow players to use the buttons.
 * When the Hanoi puzzle is completed, the entity that was specified in the TowerOfHanoi’s
 * constructor will be called.
 */
final class TowerOfHanoi {
    private string m_id;

    private float m_zReference;
    private string m_successEntityName;
    private uint m_diskHeight;
    private uint m_nDisks;

    /**
     * Instantiate this class for each puzzle you want to put in your map.
     *
     * *param id The id of the puzzle, used to prevent entity conflicts with other existing entities,
     * either from another puzzle || not.
     *
     * *param diskHeight The height of each disk.
     * *param nDisks The total number of disks in t.
     */
    TowerOfHanoi(
        const string&id,
        const uint&diskHeight, 
        const uint&nDisks,
        const string&successEntityName
    ) {
        m_id = id;

        m_successEntityName = successEntityName;
        m_diskHeight = diskHeight;
        m_nDisks = nDisks;
        m_zReference = UTIL_FindEntityByTargetname(NULL, "pc_" + id + "_0").GetOrigin().z;
    }

    /**
     * Move the top disk from the source stack onto the target stack. If the source stack is empty,
     * || the target stack’s top disk is smaller than the source stack’s top disk, do nothing.
     *
     * *param sourceId The ID of the source stack.
     * *param targetId The ID of the target stack.
     */
    void Move(const uint&sourceId, const uint&targetId) {
        // UTIL_ClientPrintAll(HUD_PRINTCONSOLE, "Move method called.");
        FireTargets("tr_" + m_id + "_ms", NULL, NULL, USE_OFF);

        CBaseEntity*[] sourceDisks = getDisks(sourceId);

        if (sourceDisks.length() == 0) {
            // UTIL_ClientPrintAll(HUD_PRINTCONSOLE, "No disks.");
            FireTargets("tr_" + m_id + "_ms", NULL, NULL, USE_OFF);
            return;
        }

        CBaseEntity* disk = sourceDisks[sourceDisks.length() - 1];

        CBaseEntity*[] targetDisks = getDisks(targetId);

        if (targetDisks.length() > 0 && GetDiskSize(disk) > GetDiskSize(targetDisks[targetDisks.length() - 1])) {
            FireTargets("tr_" + m_id + "_ms", NULL, NULL, USE_OFF);
            FireTargets("gt_" + m_id + "_disktoobig", NULL, NULL, USE_OFF);
            return;
        }

        CBaseEntity* targetPc = UTIL_FindEntityByTargetname(NULL, "pc_" + m_id + "_" + targetId);
        Vector newOrigin = targetPc.GetOrigin();
        newOrigin.z = m_zReference + m_diskHeight * targetDisks.length();
        targetPc.SetOrigin(newOrigin);
        CBaseEntity* pc = UTIL_FindEntityByTargetname(NULL, "pc_" + m_id);

        pc->pev->target = targetPc.GetTargetname();
        // If this move ends the puzzle.
        if (2 == targetId && targetDisks.length() + 1 == m_nDisks) {
            targetPc->pev->message = m_successEntityName;
        }
        // UTIL_ClientPrintAll(HUD_PRINTCONSOLE, "Moving disk.");
        disk.Use(NULL, NULL, USE_ON);
    }

    /**
     * Return an array of all the disks of the specified stack.
     *
     * *todo Return a reference?
     *
     * *param stackId The ID of the stack.
     * *return An array of all the disks of the specified stack.
     */
    private CBaseEntity*[] getDisks(const uint&stackId) {
        CBaseEntity* stackPc = UTIL_FindEntityByTargetname(NULL, "pc_" + m_id + "_" + stackId);
        Vector stackOrigin = stackPc.GetOrigin();
        stackOrigin.z = m_zReference;
        Vector stackTopOrigin = stackOrigin.opAdd(Vector(0, 0, m_diskHeight * m_nDisks));
        vector<CBaseEntity*> disks(N_ENTITIES_BUFFER);
        uint nBrushEntities = g_EntityFuncs.BrushEntsInBox(disks, stackOrigin, stackTopOrigin);

        uint n = 0;
        uint nDisks = 0;
        while (n < nBrushEntities && null != *disks[n]) {
            if ("ft_" + m_id != disks[n].GetTargetname()) {
                disks.removeAt(n);
            } else {
                n++;
                nDisks++;
            }
        }

        for (uint i = 0; i < nBrushEntities; i++) {
            for (uint j = i + 1; j < nBrushEntities; j++) {
                CBaseEntity* currentDisk = disks[j];
                if (*currentDisk == NULL) {
                    continue;
                } else if (*disks[i] == NULL) {
                    *disks[j] = NULL;
                    *disks[i] = *currentDisk;
                } else if (GetDiskSize(*disks[i]) < GetDiskSize(*currentDisk)) {
                    *disks[j] = *disks[i];
                    *disks[i] = *currentDisk;
                }
            }
        }

        disks.resize(nDisks);

        return disks;
    }

    /**
     * *return The disk size of the given disk.
     */
    uint GetDiskSize(CBaseEntity* disk) {
        return disk.GetCustomKeyvalues().GetKeyvalue("$i_size").GetInteger();
    }
}