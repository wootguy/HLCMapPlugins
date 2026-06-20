#include "extdll.h"
#include "util.h"
#include "CDoomProjectile.h"

class CDoomRocket : public CDoomProjectile {
	void Precache() {
		pev->speed = 700;
		deathFrameStart = 8;
		deathFrameEnd = 10;
		flash_color = Vector(255, 128, 32);
		damageMin = 20;
		damageMax = 160;
		oriented = true;
		//spawnSound = "doom/dsrlaunc.wav";
		deathSound = "doom/dsbarexp.wav";
		radiusDamage = 128;
		trailSprite = "sprites/doom/puff.spr";
		pev->model = ALLOC_STRING("sprites/doom/misl.spr");

		CDoomProjectile::Precache();
	}
};

class CDoomPlasmaBall : public CDoomProjectile {
	void Precache() {
		pev->speed = 875;
		moveFrameStart = 13;
		moveFrameEnd = 14;
		deathFrameStart = 14;
		deathFrameEnd = 19;
		flash_color = Vector(64, 64, 255);
		damageMin = 5;
		damageMax = 40;
		//spawnSound = "doom/dsplasma.wav";
		deathSound = "doom/dsfirxpl.wav";
		pev->model = ALLOC_STRING("sprites/doom/bal.spr");

		CDoomProjectile::Precache();
	}
};

class CDoomBfgBall : public CDoomProjectile {
	void Precache() {
		pev->speed = 875;
		moveFrameStart = 20;
		moveFrameEnd = 21;
		deathFrameStart = 22;
		deathFrameEnd = 27;
		flash_color = Vector(64, 255, 64);
		damageMin = 100;
		damageMax = 800;
		//spawnSound = "doom/dsplasma.wav";
		deathSound = "doom/dsrxplod.wav";
		pev->model = ALLOC_STRING("sprites/doom/bal.spr");

		CDoomProjectile::Precache();
	}
};

LINK_ENTITY_TO_CLASS(doom_rocket, CDoomRocket)
LINK_ENTITY_TO_CLASS(doom_plasmaball, CDoomPlasmaBall)
LINK_ENTITY_TO_CLASS(doom_bfgball, CDoomBfgBall)