#include "extdll.h"
#include "util.h"
#include "CAmmoCustom.h"
#include "CBasePlayer.h"
#include "CDoomSprite.h"

class CDoomAmmo : public CAmmoCustom {
public:
	msprite_sv_t* headerHw;
	msprite_sv_t* headerSw;
	int modelIndexSw;

	void Spawn() override {
		CAmmoCustom::Spawn();

		modelIndexSw = PRECACHE_MODEL("sprites/doom/sw/objects.spr");
		headerHw = GET_SPRITE_PTR(pev->modelindex);
		headerSw = GET_SPRITE_PTR(modelIndexSw);
	}

	int AddToFullPack(struct entity_state_s* state, CBasePlayer* player) override {
		// software mode vanilla clients can't render sprite frames
		// with offsets in them. So now I'm faking that with centered parallel sprites and an
		// offset on the entity origin.
		if (player->m_clientRenderer == CLIENT_RENDERER_SOFTWARE && !player->IsSevenKewpClient()) {
			state->origin.z += CDoomSprite::SwFrameOffset(headerHw, headerSw, params.modelBody) * pev->scale;
			state->modelindex = modelIndexSw;
		}

		return 1;
	}
};


LINK_ENTITY_TO_CLASS(ammo_doom_bulletbox, CDoomAmmo)
LINK_ENTITY_TO_CLASS(ammo_doom_bullets, CDoomAmmo)
LINK_ENTITY_TO_CLASS(ammo_doom_cellbox, CDoomAmmo)
LINK_ENTITY_TO_CLASS(ammo_doom_cells, CDoomAmmo)
LINK_ENTITY_TO_CLASS(ammo_doom_rocket, CDoomAmmo)
LINK_ENTITY_TO_CLASS(ammo_doom_rocketbox, CDoomAmmo)
LINK_ENTITY_TO_CLASS(ammo_doom_rockets, CDoomAmmo)
LINK_ENTITY_TO_CLASS(ammo_doom_shellbox, CDoomAmmo)
LINK_ENTITY_TO_CLASS(ammo_doom_shells, CDoomAmmo)