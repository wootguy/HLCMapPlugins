#include "extdll.h"
#include "util.h"
#include "CBaseMonster.h"
#include "CZombie.h"
#include "te_effects.h"
#include "skill.h"
#include "animation.h"

class CDavid : public CZombie {
	static const char* alertSounds[3];
	const char* axeGrabSound = "aomdc/davidbad/axeGrabSound.wav";
	const char* axeHit = "aomdc/davidbad/axe_hitbody.wav";
	const char* axeMiss = "aomdc/davidbad/axe_swing.wav";
	const char* fireSpr = "sprites/xffloor.spr";
	int m_fireSprIdx;

	bool m_wieldingAxe;
	float m_nextFire;
	int m_fireLeft;

	void Spawn(void) override {
		pev->health = 4000; // real game is about 70 hits on the body, that's plenty even for co-op
		CZombie::Spawn();
	}

	const char* DisplayName() { return "David"; }

	void Precache() override {
		PRECACHE_SOUND(axeGrabSound);
		PRECACHE_SOUND(axeHit);
		PRECACHE_SOUND(axeMiss);
		PRECACHE_SOUND_ARRAY(alertSounds);
		m_fireSprIdx = PRECACHE_MODEL(fireSpr);
		pev->model = ALLOC_STRING("models/aomdc/david_monster.mdl");
		
		CZombie::Precache();
	}

	void SetActivity(Activity NewActivity) override {
		int	iSequence = ACTIVITY_NOT_AVAILABLE;

		iSequence = GetActivitySequence(NewActivity);

		m_Activity = NewActivity; // Go ahead and set this so it doesn't keep trying when the anim is not present

		// Set to the desired anim, or default anim if the desired is not present
		if (iSequence > ACTIVITY_NOT_AVAILABLE)
		{
			if (pev->sequence != iSequence || !m_fSequenceLoops)
			{
				pev->frame = 0;
			}

			pev->sequence = iSequence;	// Set to the reset anim (if it's there)
			ResetSequenceInfo();
			SetYawSpeed();
		}
		else
		{
			// Not available try to get default anim
			const char* actName = NewActivity < ACT_LAST ? activity_map[NewActivity].name : "Unknown";
			ALERT(at_aiconsole, "%s has no sequence for act %s (%d)\n", STRING(pev->classname), actName, NewActivity);
			pev->sequence = 0;	// Set to the reset anim (if it's there)
		}
	}

	int GetActivitySequence(Activity NewActivity) {
		int iSequence = ACTIVITY_NOT_AVAILABLE;

		switch (NewActivity)
		{
		case ACT_MELEE_ATTACK1:
			iSequence = LookupSequence(m_wieldingAxe ? "attack_axe" : "attack");
			break;
		default:
			iSequence = LookupActivity(NewActivity);
			break;
		}

		return iSequence;
	}

	void HandleAnimEvent(MonsterEvent_t* pEvent) override {
		switch (pEvent->event)
		{
		case ZOMBIE_AE_ATTACK_RIGHT:
		{
			// real game only uses the single slash. Changed it for co-op
			float dmg = m_wieldingAxe ? gSkillData.sk_zombie_dmg_both_slash : gSkillData.sk_zombie_dmg_one_slash;

			CBaseEntity* pHurt = CheckTraceHullAttack(70, dmg, DMG_SLASH);
			if (pHurt)
			{
				if (pHurt->pev->flags & (FL_MONSTER | FL_CLIENT))
				{
					pHurt->pev->punchangle.z = -18;
					pHurt->pev->punchangle.x = 5;
					pHurt->pev->velocity = pHurt->pev->velocity - gpGlobals->v_right * 100;
				}
				// Play a random attack hit sound
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, axeHit, 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			} else 
				EMIT_SOUND_DYN(ENT(pev), CHAN_WEAPON, axeMiss, 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			break;
		}
		case 1013: {
			m_wieldingAxe = !m_wieldingAxe;
			SetBodygroup(0, m_wieldingAxe ? 1 : 0);
			EMIT_SOUND_DYN(ENT(pev), CHAN_ITEM, axeGrabSound, 1.0, ATTN_NORM, 0, 100 + RANDOM_LONG(-5, 5));
			break;
		}
		default:
			CZombie::HandleAnimEvent(pEvent);
			break;
		}
	}

	int TakeDamage(entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType) {
		if (IsImmune(pevAttacker, flDamage))
			return 0;

		if (!(bitsDamageType & DMG_SHOCK))
		{
			CBaseEntity* attacker = CBaseEntity::Instance(pevAttacker);
			attacker->TakeDamage(pevInflictor, pevAttacker, flDamage*0.2f, bitsDamageType);
			return 0;
		}

		if (pev->health < pev->max_health * 0.25f) {
			if (!m_nextFire) {
				m_fireLeft = 20;
			}
			m_nextFire = gpGlobals->time;
		}

		return CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	}

	void MonsterThink() override {
		if (m_fireLeft > 0 && m_nextFire && m_nextFire < gpGlobals->time) {
			CBaseEntity* plr = NULL;
			while ((plr = UTIL_FindEntityByClassname(plr, "player")) != NULL) {
				if ((plr->pev->origin - pev->origin).Length() < 1024 && plr->pev->waterlevel < WATERLEVEL_HEAD)
					plr->TakeDamage(pev, pev, 4, DMG_BURN);
			}

			MAKE_VECTORS(pev->angles);

			UTIL_Explosion(pev->origin + Vector(0, 0, 48) + gpGlobals->v_forward*8, m_fireSprIdx, 10, 15, 2 | 4 | 8);

			m_nextFire = gpGlobals->time + 0.5f;
			m_fireLeft--;
		}

		CBaseMonster::MonsterThink();
	}

	void AlertSound(void) override {
		int pitch = 95 + RANDOM_LONG(0, 9);
		EMIT_SOUND_DYN(ENT(pev), CHAN_VOICE, RANDOM_SOUND_ARRAY(alertSounds), 1.0, ATTN_NORM, 0, pitch);
	}

	void PainSound() override {}
	void IdleSound() override {}
};

const char* CDavid::alertSounds[3] = {
	"aomdc/davidbad/alert10.wav",
	"aomdc/davidbad/alert20.wav",
	"aomdc/davidbad/alert30.wav",
};

LINK_ENTITY_TO_CLASS(monster_david, CDavid)
