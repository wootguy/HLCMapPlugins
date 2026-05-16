#include "extdll.h"
#include "util.h"
#include "CWeaponCustom.h"
#include "te_effects.h"
#include "skill.h"

// TODO: The hands have fucked up normals and appear flatshaded.
// Same with the original model and vanilla renderer. Why?

enum ElitesAnimation {
	ELITES_IDLE = 0,
	ELITES_IDLE_LEFTEMPTY,
	ELITES_SHOOTLEFT1,
	ELITES_SHOOTLEFT2,
	ELITES_SHOOTLEFT3,
	ELITES_SHOOTLEFT4,
	ELITES_SHOOTLEFT5,
	ELITES_SHOOTLEFTLAST,
	ELITES_SHOOTRIGHT1,
	ELITES_SHOOTRIGHT2,
	ELITES_SHOOTRIGHT3,
	ELITES_SHOOTRIGHT4,
	ELITES_SHOOTRIGHT5,
	ELITES_SHOOTRIGHTLAST,
	ELITES_RELOAD,
	ELITES_DRAW
};

class CDualGlock : public CWeaponCustom {
	void WeaponIdleCustom() {
		if (m_iClip == 1) {
			SendWeaponAnim(ELITES_IDLE_LEFTEMPTY, -1);
			m_flTimeWeaponIdle = 9999; // prevent default idle anim
		}
	}
};


LINK_ENTITY_TO_CLASS(weapon_dualglock, CDualGlock);