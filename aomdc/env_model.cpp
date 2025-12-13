#include "extdll.h"
#include "util.h"
#include "CBaseEntity.h"

class CEnvModel : public CBaseEntity
{
	void Spawn(void)
	{
		Precache();
		SET_MODEL(ENT(pev), STRING(pev->model));

		if (pev->spawnflags & 2) {
			pev->movetype == MOVETYPE_TOSS;
			SetThink(&CEnvModel::DropThink);
			pev->nextthink = gpGlobals->time;
		}
		else {
			pev->solid = SOLID_NOT;
			pev->movetype = MOVETYPE_NONE;
		}
		
		UTIL_SetSize(pev, Vector(0, 0, 0), Vector(0, 0, 0));
	}

	void Precache(void) {
		PRECACHE_MODEL(STRING(pev->model));
	}

	void DropThink() {
		int dropResult = DROP_TO_FLOOR(ENT(pev));

		if (dropResult == 0)
		{
			ALERT(at_warning, "Model %s fell out of level at %f,%f,%f\n", STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z);
			UTIL_Remove(this);
			return;
		}
		else if (dropResult == -1) {
			ALERT(at_warning, "Model %s spawned inside solid at %f,%f,%f\n", STRING(pev->classname), pev->origin.x, pev->origin.y, pev->origin.z);
		}

		SetThink(NULL);
	}
};

LINK_ENTITY_TO_CLASS(env_model, CEnvModel)