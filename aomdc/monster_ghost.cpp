#include "extdll.h"
#include "util.h"
#include "CISlave.h"
#include "CBasePlayer.h"
#include "te_effects.h"
#include "aomdc.h"
#include "shake.h"
#include "Scheduler.h"

void GhostAttack(int pidx, EHANDLE attacker, int damageTicks) {
	PlayerState& state = g_playerStates[pidx];
	CBasePlayer* plr = UTIL_PlayerByIndex(pidx);

	if (!plr || !state.ghostDamage)
		return;

	if (!attacker)
		attacker = plr;

	plr->TakeDamage(plr->pev, plr->pev, 4, DMG_GENERIC);

	if (--damageTicks > 0 && plr->IsAlive()) {
		g_Scheduler.SetTimeout(GhostAttack, 2.0f, pidx, attacker, damageTicks);
	}
	else {
		state.ghostDamage = false;
	}
}

class CGhost : public CISlave {
	const char* earRingingSnd = "aomdc/ghost/ear_ringing.wav";

	void Spawn(void) override {
		pev->classname = ALLOC_STRING("monster_alien_slave");
		CISlave::Spawn();
	}

	const char* DisplayName() { return "Ghost"; }

	void Precache() override {
		PRECACHE_SOUND(earRingingSnd);
		pev->model = ALLOC_STRING("models/aomdc/ghost.mdl");
		CISlave::Precache();
	}

	BOOL CheckRangeAttack1(float flDot, float flDist) override {
		return FALSE;
	}

	BOOL CheckMeleeAttack1(float flDot, float flDist) override {

		if (flDist < 96) {
			CBasePlayer* plr = m_hEnemy ? m_hEnemy->MyPlayerPointer() : NULL;

			if (plr) {
				PlayerState& state = getPlayerState(plr);

				if (!state.ghostDamage) {
					state.ghostDamage = true;
					GhostAttack(plr->entindex(), EHANDLE(edict()), 6);
					UTIL_ScreenFade(plr, Vector(255, 0, 0), 2, 10, 128, FFADE_IN);
					StartSound(plr->entindex(), CHAN_STATIC, earRingingSnd, 1, 0, 0, 100,
						plr->pev->origin, PLRBIT(plr->edict()));
				}
			}
		}

		return FALSE;
	}

	void PainSound(void) override {}
	void AlertSound(void) override {}
	void IdleSound(void) override {}
};

LINK_ENTITY_TO_CLASS(monster_ghost, CGhost)
