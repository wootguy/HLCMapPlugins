#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

class CHammer : public CWeaponCustom {
	bool MeleeHit(CBasePlayer* plr, CBaseEntity* target) override {
		UTIL_ScreenShake(plr, 6, 5, 0.7f);
		return false;
	}
};

LINK_ENTITY_TO_CLASS(weapon_hammer, CHammer)