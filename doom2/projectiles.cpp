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
		is_bfg = true;

		PRECACHE_MODEL("sprites/doom/bfe2.spr");

		CDoomProjectile::Precache();
	}
};

class CDoomFireBall : public CDoomProjectile {
	void Precache() {
		pev->speed = 350;
		moveFrameStart = 0;
		moveFrameEnd = 1;
		deathFrameStart = 2;
		deathFrameEnd = 4;
		flash_color = Vector(255, 54, 32);
		damageMin = 3;
		damageMax = 24;
		deathSound = "doom/dsfirxpl.wav";
		pev->model = ALLOC_STRING("sprites/doom/bal.spr");

		CDoomProjectile::Precache();
	}
};

class CDoomCacoBall : public CDoomProjectile {
	void Precache() {
		pev->speed = 350;
		moveFrameStart = 5;
		moveFrameEnd = 6;
		deathFrameStart = 7;
		deathFrameEnd = 9;
		flash_color = Vector(255, 32, 64);
		damageMin = 5;
		damageMax = 40;
		deathSound = "doom/dsfirxpl.wav";
		pev->model = ALLOC_STRING("sprites/doom/bal.spr");

		CDoomProjectile::Precache();
	}
};

class CDoomBaronBall : public CDoomProjectile {
	void Precache() {
		pev->speed = 525;
		moveFrameStart = 0;
		moveFrameEnd = 1;
		deathFrameStart = 16;
		deathFrameEnd = 18;
		flash_color = Vector(64, 255, 64);
		damageMin = 8;
		damageMax = 64;
		deathSound = "doom/dsfirxpl.wav";
		pev->model = ALLOC_STRING("sprites/doom/bal7.spr");
		oriented = true;

		CDoomProjectile::Precache();
	}
};

class CArchVileFire : public CDoomProjectile {
	void Precache() {
		pev->speed = 0;
		moveFrameStart = 0;
		moveFrameEnd = 7;
		deathFrameStart = 7;
		deathFrameEnd = 7;
		flash_color = Vector(255, 255, 64);
		damageMin = 20;
		damageMax = 64;
		is_vile_fire = 64;
		radiusDamage = 70;
		deathSound = "doom/dsbarexp.wav";
		pev->model = ALLOC_STRING("sprites/doom/fire.spr");
		oriented = true;

		CDoomProjectile::Precache();
	}
};

class CRevenantBall : public CDoomProjectile {
	void Precache() {
		pev->speed = 350;
		moveFrameStart = 0;
		moveFrameEnd = 1;
		deathFrameStart = 16;
		deathFrameEnd = 18;
		flash_color = Vector(255, 64, 32);
		damageMin = 10;
		damageMax = 80;
		deathSound = "doom/dsbarexp.wav";
		trailSprite = "sprites/doom/puff.spr";
		pev->model = ALLOC_STRING("sprites/doom/fatb.spr");
		oriented = true;

		CDoomProjectile::Precache();
	}
};

class CMancubusBall : public CDoomProjectile {
	void Precache() {
		pev->speed = 700;
		moveFrameStart = 0;
		moveFrameEnd = 0;
		deathFrameStart = 16;
		deathFrameEnd = 18;
		flash_color = Vector(255, 80, 32);
		damageMin = 8;
		damageMax = 64;
		deathSound = "doom/dsbarexp.wav";
		pev->model = ALLOC_STRING("sprites/doom/manf.spr");
		oriented = true;

		CDoomProjectile::Precache();
	}
};

class CSpiderBall : public CDoomProjectile {
	void Precache() {
		pev->speed = 875;
		moveFrameStart = 28;
		moveFrameEnd = 29;
		deathFrameStart = 30;
		deathFrameEnd = 34;
		flash_color = Vector(32, 255, 32);
		damageMin = 5;
		damageMax = 40;
		deathSound = "doom/dsbarexp.wav";
		pev->model = ALLOC_STRING("sprites/doom/bal.spr");

		CDoomProjectile::Precache();
	}
};

LINK_ENTITY_TO_CLASS(doom_rocket, CDoomRocket)
LINK_ENTITY_TO_CLASS(doom_plasmaball, CDoomPlasmaBall)
LINK_ENTITY_TO_CLASS(doom_bfgball, CDoomBfgBall)
LINK_ENTITY_TO_CLASS(doom_fireball, CDoomFireBall)
LINK_ENTITY_TO_CLASS(doom_cacoball, CDoomCacoBall)
LINK_ENTITY_TO_CLASS(doom_baronball, CDoomBaronBall)
LINK_ENTITY_TO_CLASS(doom_vilefire, CArchVileFire)
LINK_ENTITY_TO_CLASS(doom_revball, CRevenantBall)
LINK_ENTITY_TO_CLASS(doom_mancuball, CMancubusBall)
LINK_ENTITY_TO_CLASS(doom_spiderball, CSpiderBall)