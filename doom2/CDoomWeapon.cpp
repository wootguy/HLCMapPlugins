#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "CDoomSprite.h"
#include "doom2.h"

using namespace std;

class CDoomWeapon : public CWeaponCustom {
public:
	msprite_sv_t* headerHw;
	msprite_sv_t* headerSw;
	int modelIndexSw;

	void Spawn() override {
		CWeaponCustom::Spawn();

		modelIndexSw = PRECACHE_MODEL("sprites/doom/sw/objects.spr");
		headerHw = GET_SPRITE_PTR(pev->modelindex);
		headerSw = GET_SPRITE_PTR(modelIndexSw);
	}

	virtual int AddToPlayer(CBasePlayer* pPlayer) override {
		int ret = CWeaponCustom::AddToPlayer(pPlayer);

		if (ret) {
			static unordered_map<string, int> unlock_types = {
				{"weapon_doom_pistol", 0},
				{"weapon_doom_fist", 0},
				{"weapon_doom_chainsaw", UNLOCK_CHAINSAW},
				{"weapon_doom_shotgun", UNLOCK_SHOTGUN},
				{"weapon_doom_supershotgun", UNLOCK_SUPER_SHOTGUN},
				{"weapon_doom_chaingun", UNLOCK_CHAINGUN},
				{"weapon_doom_rpg", UNLOCK_RPG},
				{"weapon_doom_plasmagun", UNLOCK_PLASMA},
				{"weapon_doom_bfg", UNLOCK_BFG},
			};

			auto unlockId = unlock_types.find(STRING(pev->classname));
			if (unlockId != unlock_types.end()) {
				unlock_item(true, unlockId->second);
			}
			else {
				ALERT(at_error, "unknown unlock for weapon %s\n", STRING(pev->classname));
			}
		}

		return ret;
	}

	int AddToFullPack(struct entity_state_s* state, CBasePlayer* player) override {
		// software mode vanilla clients can't render sprite frames
		// with offsets in them. So now I'm faking that with centered parallel sprites and an
		// offset on the entity origin.
		if (player->m_clientRenderer == CLIENT_RENDERER_SOFTWARE && !player->IsSevenKewpClient()) {
			state->origin.z += CDoomSprite::SwFrameOffset(headerHw, headerSw, defaultParams.wsprite_frame) * pev->scale;
			state->modelindex = modelIndexSw;
		}

		return 1;
	}
};


LINK_ENTITY_TO_CLASS(weapon_doom_fist, CDoomWeapon)
LINK_ENTITY_TO_CLASS(weapon_doom_pistol, CDoomWeapon)
LINK_ENTITY_TO_CLASS(weapon_doom_shotgun, CDoomWeapon)
LINK_ENTITY_TO_CLASS(weapon_doom_supershotgun, CDoomWeapon)
LINK_ENTITY_TO_CLASS(weapon_doom_chainsaw, CDoomWeapon)
LINK_ENTITY_TO_CLASS(weapon_doom_rpg, CDoomWeapon)
LINK_ENTITY_TO_CLASS(weapon_doom_chaingun, CDoomWeapon)
LINK_ENTITY_TO_CLASS(weapon_doom_plasmagun, CDoomWeapon)
LINK_ENTITY_TO_CLASS(weapon_doom_bfg, CDoomWeapon)