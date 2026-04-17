#include "CBaseEntity.h"

#define S_PI 3.141592653589793
#define EN_FT_STATIC_DR_0 "ft_ac_dr_0"
#define EN_FT_STATIC_DR_1 "ft_ac_dr_1"
#define EN_FTT_DYNAMIC_CART "ftt_ac"
#define EN_FW_STATIC_STRUCTURE "fw_ac_struc"
#define EN_IT_STORAGE "it_tankstorage"
#define EN_MM_ON_STOP_IN_B "mm_ac_onstop_b"
#define EN_PC_STATIC_DR_0_0 "pc_ac_dr_0_0"
#define EN_PC_STATIC_DR_0_1 "pc_ac_dr_0_1"
#define EN_PC_STATIC_DR_1_0 "pc_ac_dr_1_0"
#define EN_PC_STATIC_DR_1_1 "pc_ac_dr_1_1"

#define INFO_MSG "info"
#define SUCCESS_MSG "success"
#define WARNING_MSG "warning"

#define EN_MM_ON_TNL_DETECTION "_mm_tnl_lsrdetect"
#define EN_MM_ON_WB_DETECTION "_mm_wb_lsrdetect"

hudtextparms_t CreateHudTextParams(const char* tone, int channel);

class Cart
{
public:
    EHANDLE m_ftDr0;
    EHANDLE m_ftDr1;
    EHANDLE m_fttDynamicCart;
    EHANDLE m_fwStaticStructure;
    EHANDLE m_pcDr0_0;
    EHANDLE m_pcDr0_1;
    EHANDLE m_pcDr1_0;
    EHANDLE m_pcDr1_1;
    const char* m_sCartId;
    Vector m_vRelOriginFtDr0;
    Vector m_vRelOriginFtDr1;
    Vector m_vRelOriginPcDr0_0;
    Vector m_vRelOriginPcDr0_1;
    Vector m_vRelOriginPcDr1_0;
    Vector m_vRelOriginPcDr1_1;
    Vector m_vStorage;

    Cart() {}

    Cart(const char* sCartId);

    void SwapStaticForDyn();

    void SwapDynForStatic();

    void WaitForTrainToStop(const char* target);

    // *todo Make private
    double ToRad(const double degree);

    // *todo Make private
    Vector RotatePosition(Vector initialPosition, const double angle_rad);
};

class LaserActivation
{
public:
    EHANDLE m_activator;
    const char* m_target;

    LaserActivation(CBaseEntity* activator, const char* target);

    void React();

    // *todo make private
    bool AreTurretsAllies();

    // *todo make private
    void NameShame();
};

#define N_ENTITIES_BUFFER 32

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
class TowerOfHanoi {
public:
    const char* m_id;

    float m_zReference;
    const char* m_successEntityName;
    int m_diskHeight;
    int m_nDisks;

    TowerOfHanoi() {}

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
        const char* id,
        const int diskHeight,
        const int nDisks,
        const char* successEntityName
    );

    /**
     * Move the top disk from the source stack onto the target stack. If the source stack is empty,
     * || the target stack’s top disk is smaller than the source stack’s top disk, do nothing.
     *
     * *param sourceId The ID of the source stack.
     * *param targetId The ID of the target stack.
     */
    void Move(const int sourceId, const int targetId);

    /**
     * Return an array of all the disks of the specified stack.
     *
     * *todo Return a reference?
     *
     * *param stackId The ID of the stack.
     * *return An array of all the disks of the specified stack.
     */
    std::vector<CBaseEntity*> getDisks(const int stackId);

    /**
     * *return The disk size of the given disk.
     */
    int GetDiskSize(CBaseEntity* disk);
};