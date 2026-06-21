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

	void Spawn() override;
	void Precache() override;
	virtual int AddToFullPack(struct entity_state_s* state, CBasePlayer* player);
};
