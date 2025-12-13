#include "extdll.h"
#include "util.h"
#include "CBaseMonster.h"
#include "CZombie.h"
#include "CBullsquid.h"
#include "CHeadCrab.h"
#include "CHoundeye.h"
#include "CController.h"
#include "CAGrunt.h"
#include "te_effects.h"
#include "aomdc.h"

class CAomZombie : public CZombie {
	void Spawn(void) override {
		pev->classname = ALLOC_STRING("monster_zombie");
		CZombie::Spawn();
	}
};

class CAomZombie2 : public CAomZombie {
	void Precache() override {
		pev->model = ALLOC_STRING("models/aomdc/zombie2.mdl");
		pev->health = gSkillData.sk_zombie_health * 1.2f;
		CZombie::Precache();
	}
};

class CAomZombie3 : public CAomZombie {
	void Precache() override {
		pev->model = ALLOC_STRING("models/aomdc/zombie3.mdl");
		pev->health = gSkillData.sk_zombie_health * 1.3f;
		CZombie::Precache();
	}
};

class CAomZombie4 : public CAomZombie {
	void Precache() override {
		pev->model = ALLOC_STRING("models/aomdc/zombie4.mdl");
		CZombie::Precache();
	}
};

class CAomBullSquid : public CBullsquid {
	void Spawn(void) override {
		CBullsquid::Spawn();
	}
};

class CAomHeadCrab : public CHeadCrab {
	void Spawn(void) override {
		CHeadCrab::Spawn();
	}
	const char* DisplayName() { return "Head"; }
};

class CAomHoundeye : public CHoundeye {
	void Spawn(void) override {
		CHoundeye::Spawn();
	}
	const char* DisplayName() override { return "Dog"; }
};

class CAomController : public CController {
	void Spawn(void) override {
		CController::Spawn();
	}
	const char* DisplayName() override { return "Face"; }
};

class CAomAGrunt : public CAGrunt {
	void Spawn(void) override {
		CAGrunt::Spawn();
	}
	
	const char* DisplayName() override { return "Face"; }

	void TraceAttack(entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult* ptr, int bitsDamageType) override {
		CBaseMonster::TraceAttack(pevAttacker, flDamage, vecDir, ptr, bitsDamageType);
	}
};

LINK_ENTITY_TO_CLASS(monster_zombie, CAomZombie)
LINK_ENTITY_TO_CLASS(monster_zombie2, CAomZombie2)
LINK_ENTITY_TO_CLASS(monster_zombie3, CAomZombie3)
LINK_ENTITY_TO_CLASS(monster_zombie4, CAomZombie4)
LINK_ENTITY_TO_CLASS(monster_bullchicken, CAomBullSquid)
LINK_ENTITY_TO_CLASS(monster_headcrab, CAomHeadCrab)
LINK_ENTITY_TO_CLASS(monster_houndeye, CAomHoundeye)
LINK_ENTITY_TO_CLASS(monster_alien_controller, CAomController)
LINK_ENTITY_TO_CLASS(monster_alien_grunt, CAomAGrunt)
