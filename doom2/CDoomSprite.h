#pragma once
#include "CPointEntity.h"

// a sprite that renders a different frame depending on your view point
class CDoomSprite : public CBaseEntity {
public:
	bool oriented; // if true, sprite displays a different frame depending on view point
	int animOffset; // frame offset for current animation
	int animFrames; // total frames of animation for group

	void Spawn() override;
	void Precache() override;
	virtual int AddToFullPack(struct entity_state_s* state, CBasePlayer* player);
	int GetSpriteAngle(Vector spritePos, Vector spriteForward, Vector spriteRight, Vector lookPos);
};