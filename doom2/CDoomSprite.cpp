#include "extdll.h"
#include "util.h"
#include "CBasePlayer.h"
#include "CDoomSprite.h"
#include "doom_utils.h"

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

void CDoomSprite::Spawn() {
	pev->solid = SOLID_NOT;
	pev->movetype = MOVETYPE_NONE;

	Precache();
	SET_MODEL(ENT(pev), STRING(pev->model));
	UTIL_SetSize(pev, g_vecZero, g_vecZero);
}

void CDoomSprite::Precache() {
	if (pev->model)
		PRECACHE_MODEL((char*)STRING(pev->model));
}

int CDoomSprite::AddToFullPack(struct entity_state_s* state, CBasePlayer* player) {
	state->movetype = MOVETYPE_NOCLIP; // interpolate without running movement code

	if (oriented) {
		int angleIdx = GetSpriteAngle(pev->origin, forwardDir, rightDir, player->pev->origin);
		state->frame = pev->frame * 8 + angleIdx;
	}
	if (player->m_clientRenderer == CLIENT_RENDERER_SOFTWARE) {
		state->origin = pev->aiment ? pev->aiment->v.origin + Vector(0,0,32) : state->origin;
		state->aiment = 0;
		state->movetype = MOVETYPE_NONE;
		if (modelIndexSw) {
			state->modelindex = modelIndexSw;
		}
	}

	return 1;
}

int CDoomSprite::SwFrameOffset(msprite_sv_t* headerHw, msprite_sv_t* headerSw, int frame) {
	if (!headerHw || !headerSw) {
		ALERT(at_error, "Missing sprite headers\n");
		return 0;
	}

	frame = clamp(frame, 0, V_min(headerHw->numframes, headerSw->numframes) - 1);
	mspriteframe_sv_t* frameHw = headerHw->frames[frame].frameptr_sv;
	mspriteframe_sv_t* frameSw = headerSw->frames[frame].frameptr_sv;
	return frameHw->up - frameSw->up;
}

LINK_ENTITY_TO_CLASS(doom_sprite, CDoomSprite)