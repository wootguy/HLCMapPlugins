#include "extdll.h"
#include "util.h"
#include "CBaseMonster.h"
#include "CZombie.h"
#include "te_effects.h"
#include "skill.h"

class CMonsterWheelchair : public CZombie {
	const char* attackSound = "aomdc/wheelchair/wcm_squirt.wav";
	const char* vomitSprite = "sprites/aomdc/wheelchair_vomit.spr";
	static const char* wheelSounds[4];

	int m_vomitSpriteIdx;

	void Spawn(void) override {
		pev->classname = ALLOC_STRING("monster_zombie");
		pev->health = gSkillData.sk_zombie_health * 1.6f;
		CZombie::Spawn();
	}

	const char* DisplayName() { return "Wheelchair Monster"; }

	void Precache() override {
		PRECACHE_SOUND(attackSound);
		PRECACHE_SOUND_ARRAY(wheelSounds);
		m_vomitSpriteIdx = PRECACHE_MODEL(vomitSprite);
		pev->model = ALLOC_STRING("models/aomdc/wheelchair_monster.mdl");
		CZombie::Precache();
	}

	void HandleAnimEvent(MonsterEvent_t* pEvent) override {
		switch (pEvent->event)
		{
		case 1: {
			Vector headOri = pev->origin + Vector(0, 0, 48);
			EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, attackSound, 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			MESSAGE_BEGIN(MSG_PVS, SVC_TEMPENTITY, headOri);
			WRITE_BYTE(TE_BLOODSPRITE);
			WRITE_COORD(headOri.x);								// pos
			WRITE_COORD(headOri.y);
			WRITE_COORD(headOri.z);
			WRITE_SHORT(m_vomitSpriteIdx);				// initial sprite model
			WRITE_SHORT(m_vomitSpriteIdx);
			WRITE_BYTE(mp_blood_color_human.value);
			WRITE_BYTE(8);		// size
			MESSAGE_END();
			break;
		}
		case 1011:
			EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, RANDOM_SOUND_ARRAY(wheelSounds), 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			break;
		}
		
		CZombie::HandleAnimEvent(pEvent);
	}
};

const char* CMonsterWheelchair::wheelSounds[4] = {
	"aomdc/wheelchair/wheel01.wav",
	"aomdc/wheelchair/wheel02.wav",
	"aomdc/wheelchair/wheel03.wav",
	"aomdc/wheelchair/wheel04.wav",
};

LINK_ENTITY_TO_CLASS(monster_wheelchair, CMonsterWheelchair)
