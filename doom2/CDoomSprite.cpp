#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "CDoomSprite.h"
#include "doom_utils.h"

void CDoomSprite::Spawn() {
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	Precache();
	SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetSize(pev, g_vecZero, g_vecZero);
}

void CDoomSprite::Precache() {
	PRECACHE_MODEL((char*)STRING(pev->model));
}

int CDoomSprite::GetSpriteAngle(Vector spritePos, Vector spriteForward, Vector spriteRight, Vector lookPos)
{
	Vector delta = spritePos - lookPos;
	delta.z = 0;
	delta = delta.Normalize();

	float dot = DotProduct(spriteForward, delta);
	float dotR = DotProduct(spriteRight, delta);

	//println("DOTR: " + dotR);

	if (dot < -0.9f)
		return 0;
	else if (dot < -0.4f)
		return dotR > 0 ? 1 : 7;
	else if (dot < 0.4f)
		return dotR > 0 ? 2 : 6;
	else if (dot < 0.9f)
		return dotR > 0 ? 3 : 5;

	return 4;
}

int CDoomSprite::AddToFullPack(struct entity_state_s* state, CBasePlayer* player) {
	state->movetype = MOVETYPE_NOCLIP; // interpolate without running movement code

	if (oriented) {
		g_engfuncs.pfnMakeVectors(pev->angles);
		Vector forward = gpGlobals->v_forward;
		Vector right = gpGlobals->v_right;
		int angleIdx = GetSpriteAngle(pev->origin, forward, right, player->pev->origin);

		state->frame = pev->frame * 8 + angleIdx;
	}
	
	return 1;
}