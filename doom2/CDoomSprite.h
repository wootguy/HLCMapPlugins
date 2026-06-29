#pragma once
#include "CPointEntity.h"
#include "CBaseMonster.h"

// a sprite that renders a different frame depending on your view point
class CDoomSprite : public CBaseEntity {
public:
	// for faster AddToFullPack logic
	Vector forwardDir;
	Vector rightDir;
	bool oriented;		// if true, sprite displays a different frame depending on view point
	int modelIndexSw; // software mode modelIdx (0 = use hw sprite)

	void Spawn() override;
	void Precache() override;
	virtual int AddToFullPack(struct entity_state_s* state, CBasePlayer* player);

	static int GetSpriteAngle(Vector spritePos, Vector spriteForward, Vector spriteRight, Vector lookPos);
	
	// calculate how much Z offset to apply to an entity for software mode vanilla clients who
	// can't properly render sprite frames with offsets in them.
	// headerHw = hardware mode sprite with the offset baked in
	// headerSw = software mode sprite which has no offset
	static int SwFrameOffset(msprite_sv_t* headerHw, msprite_sv_t* headerSw, int frame);
};
