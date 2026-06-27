#include "extdll.h"
#include "util.h"
#include "CDoomMonster.h"
#include "CDoomProjectile.h"
#include "doom_utils.h"
#include "hlds_hooks.h"

class CImp : public CDoomMonster
{
	void Spawn()
	{
		bodySpriteName = "troo";

		animInfo.push_back(AnimInfo(0, 1, 0.125f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 3, 0.25f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(4, 6, 0.25f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(4, 6, 0.25f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(7, 7, 0.125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(64, 68, 0.25f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(69, 76, 0.5f, false)); // ANIM_GIB		

		idleSounds.push_back("doom/dsbgact.wav");
		painSound = "doom/dspopain.wav";
		deathSounds.push_back("doom/dsbgdth1.wav");
		deathSounds.push_back("doom/dsbgdth2.wav");
		alertSounds.push_back("doom/dsbgsit1.wav");
		alertSounds.push_back("doom/dsbgsit2.wav");
		meleeSound = "doom/dsclaw.wav";

		this->hasMelee = true;
		this->hasRanged = true;
		this->painChance = 0.78f;

		m_displayName = ALLOC_STRING("Imp");
		pev->health = 60;

		DoomSpawn();

		SetThink(&CImp::Think);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	void MeleeAttack(Vector aimDir)
	{
		if (Slash(aimDir, RANDOM_LONG(3, 24)))
			EMIT_SOUND_DYN(edict(), CHAN_WEAPON, meleeSound, 1.0f, 0.5f, 0, 100);
	}

	void RangeAttack(Vector aimDir)
	{
		brighten = 8;

		Vector bodyPos = BodyPos();
		Vector angles = UTIL_VecToAngles(aimDir);

		CBaseEntity* ent = CBaseEntity::Create("doom_fireball", bodyPos, angles, true, edict());
		EMIT_SOUND_DYN(ent->edict(), CHAN_BODY, "doom/dsfirsht.wav", 1.0f, ATTN_NORM, 0, 100);
	}

	void Think()
	{
		DoomThink();
	}
};

class CZombieMan : public CDoomMonster
{
	void Spawn()
	{
		bodySpriteName = "poss";

		animInfo.push_back(AnimInfo(0, 1, 0.125f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 3, 0.25f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(4, 4, 0.25f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(4, 4, 0.25f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(6, 6, 0.125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(56, 60, 0.25f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(61, 69, 0.5f, false)); // ANIM_GIB		

		animInfo[ANIM_ATTACK].attackFrames.resize(0);
		animInfo[ANIM_ATTACK].attackFrames.push_back(2);
		animInfo[ANIM_ATTACK].frameIndices.push_back(4);
		animInfo[ANIM_ATTACK].frameIndices.push_back(5);
		animInfo[ANIM_ATTACK].frameIndices.push_back(4);
		animInfo[ANIM_ATTACK2] = animInfo[ANIM_ATTACK];

		idleSounds.push_back("doom/dsposact.wav");
		painSound = "doom/dspopain.wav";
		deathSounds.push_back("doom/dspodth2.wav");
		deathSounds.push_back("doom/dspodth3.wav");
		deathSounds.push_back("doom/dspodth1.wav");
		alertSounds.push_back("doom/dsposit1.wav");
		alertSounds.push_back("doom/dsposit2.wav");
		alertSounds.push_back("doom/dsposit3.wav");
		shootSound = "doom/dspistol.wav";

		this->hasMelee = false;
		this->hasRanged = true;
		this->painChance = 0.78f;
		this->dropItem = "ammo_doom_bullets";

		m_displayName = ALLOC_STRING("Zombie Man");
		pev->health = 20;

		DoomSpawn();

		SetThink(&CZombieMan::Think);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	void RangeAttack(Vector aimDir)
	{
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, shootSound, 1.0f, 0.5f, 0, 100);

		ShootBullet(aimDir, 22.0f, RANDOM_LONG(3, 15));
	}

	void Think()
	{
		DoomThink();
	}
};

class CShotgunGuy : public CDoomMonster
{
	void Spawn()
	{
		bodySpriteName = "spos";

		animInfo.push_back(AnimInfo(0, 1, 0.125f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 3, 0.25f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(4, 4, 0.25f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(4, 4, 0.25f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(6, 6, 0.125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(56, 60, 0.25f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(61, 69, 0.5f, false)); // ANIM_GIB		

		animInfo[ANIM_ATTACK].attackFrames.resize(0);
		animInfo[ANIM_ATTACK].attackFrames.push_back(2);
		animInfo[ANIM_ATTACK].frameIndices.push_back(4);
		animInfo[ANIM_ATTACK].frameIndices.push_back(5);
		animInfo[ANIM_ATTACK].frameIndices.push_back(4);
		animInfo[ANIM_ATTACK2] = animInfo[ANIM_ATTACK];

		idleSounds.push_back("doom/dsposact.wav");
		painSound = "doom/dspopain.wav";
		deathSounds.push_back("doom/dspodth2.wav");
		deathSounds.push_back("doom/dspodth3.wav");
		deathSounds.push_back("doom/dspodth1.wav");
		alertSounds.push_back("doom/dsposit1.wav");
		alertSounds.push_back("doom/dsposit2.wav");
		shootSound = "doom/dsshotgn.wav";

		this->hasMelee = false;
		this->hasRanged = true;
		this->painChance = 0.66f;
		this->dropItem = "weapon_doom_shotgun";

		m_displayName = ALLOC_STRING("Shotgun Guy");
		pev->health = 30;

		DoomSpawn();

		SetThink(&CShotgunGuy::Think);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	void RangeAttack(Vector aimDir)
	{
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, shootSound, 1.0f, 0.5f, 0, 100);

		ShootBullet(aimDir, 22.0f, RANDOM_LONG(3, 15));
		ShootBullet(aimDir, 22.0f, RANDOM_LONG(3, 15), false);
		ShootBullet(aimDir, 22.0f, RANDOM_LONG(3, 15), false);
	}

	void Think()
	{
		DoomThink();
	}
};

class CHwDude : public CDoomMonster
{
	void Spawn()
	{
		bodySpriteName = "cpos";

		animInfo.push_back(AnimInfo(0, 1, 0.125f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 3, 0.25f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(4, 5, 0.5f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(4, 5, 0.5f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(6, 6, 0.125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(56, 62, 0.5f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(63, 68, 0.5f, false)); // ANIM_GIB		

		animInfo[ANIM_ATTACK].attackFrames.resize(0);
		animInfo[ANIM_ATTACK].attackFrames.push_back(3);
		animInfo[ANIM_ATTACK].attackFrames.push_back(4);
		animInfo[ANIM_ATTACK].frameIndices.insert(animInfo[ANIM_ATTACK].frameIndices.begin(), 4);
		animInfo[ANIM_ATTACK].frameIndices.insert(animInfo[ANIM_ATTACK].frameIndices.begin(), 4);
		animInfo[ANIM_ATTACK].frameIndices.insert(animInfo[ANIM_ATTACK].frameIndices.begin(), 4);
		animInfo[ANIM_ATTACK2] = animInfo[ANIM_ATTACK];
		this->constantAttackLoopFrame = 3;
		//animInfo[ANIM_ATTACK].frameIndices.push_back(5);
		//animInfo[ANIM_ATTACK].frameIndices.push_back(4);

		idleSounds.push_back("doom/dsposact.wav");
		painSound = "doom/dspopain.wav";
		deathSounds.push_back("doom/dspodth2.wav");
		deathSounds.push_back("doom/dspodth3.wav");
		deathSounds.push_back("doom/dspodth1.wav");
		alertSounds.push_back("doom/dsposit1.wav");
		alertSounds.push_back("doom/dsposit2.wav");
		alertSounds.push_back("doom/dsposit3.wav");
		shootSound = "doom/dsshotgn.wav";

		this->hasMelee = false;
		this->hasRanged = true;
		this->painChance = 0.66f;
		this->constantAttack = true;
		this->hullModel = "models/doom/null_wide.mdl";

		this->dropItem = "weapon_doom_chaingun";

		m_displayName = ALLOC_STRING("Heavy Weapon Dude");
		pev->health = 70;

		DoomSpawn();

		SetThink(&CHwDude::Think);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	void RangeAttack(Vector aimDir)
	{
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, shootSound, 1.0f, 0.5f, 0, 100);

		ShootBullet(aimDir, 22.0f, RANDOM_LONG(3, 15));
	}

	void Think()
	{
		DoomThink();
	}
};

class CDemon : public CDoomMonster
{
	void Spawn()
	{
		bodySpriteName = "sarg";

		walkSpeed = 10.0f;

		animInfo.push_back(AnimInfo(0, 1, 0.125f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 3, 0.5f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(4, 6, 0.25f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(4, 6, 0.25f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(7, 7, 0.0125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(64, 69, 0.5f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(64, 69, 0.5f, false)); // ANIM_GIB		

		animInfo[ANIM_DEAD].frameIndices.insert(animInfo[ANIM_DEAD].frameIndices.begin() + 1, 65);
		animInfo[ANIM_DEAD].frameIndices.insert(animInfo[ANIM_DEAD].frameIndices.begin(), 64);

		idleSounds.push_back("doom/dsdmact.wav");
		painSound = "doom/dsdmpain.wav";
		deathSounds.push_back("doom/dssgtdth.wav");
		alertSounds.push_back("doom/dssgtsit.wav");
		meleeSound = "doom/dssgtatk.wav";

		this->hasMelee = true;
		this->hasRanged = false;
		this->painChance = 0.7f;
		this->hullModel = "models/doom/null_wide.mdl";

		m_displayName = ALLOC_STRING("Demon");
		pev->health = 150;

		DoomSpawn();

		SetThink(&CDemon::Think);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	void MeleeAttack(Vector aimDir)
	{
		if (Slash(aimDir, RANDOM_LONG(4, 40)))
			EMIT_SOUND_DYN(edict(), CHAN_WEAPON, meleeSound, 1.0f, 0.5f, 0, 100);
	}

	void Think()
	{
		DoomThink();
	}
};

class CCacodemon : public CDoomMonster
{
	void Spawn()
	{
		bodySpriteName = "head";

		animInfo.push_back(AnimInfo(0, 0, 0.125f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 0, 0.25f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(1, 3, 0.25f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(1, 3, 0.25f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(4, 4, 0.125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(48, 53, 0.25f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(48, 53, 0.5f, false)); // ANIM_GIB		

		idleSounds.push_back("doom/dsdmact.wav");
		painSound = "doom/dsdmpain.wav";
		deathSounds.push_back("doom/dscacdth.wav");
		alertSounds.push_back("doom/dscacsit.wav");

		this->hasMelee = true;
		this->hasRanged = true;
		this->canFly = true;
		this->painChance = 0.5f;
		this->hullModel = "models/doom/null_fat.mdl";
		this->largeHull = true;

		m_displayName = ALLOC_STRING("Cacodemon");
		pev->health = 400;

		DoomSpawn();

		SetThink(&CCacodemon::Think);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	void MeleeAttack(Vector aimDir)
	{
		Slash(aimDir, RANDOM_LONG(10, 60));
	}

	void RangeAttack(Vector aimDir)
	{
		brighten = 8;

		Vector bodyPos = BodyPos();
		Vector angles = UTIL_VecToAngles(aimDir);

		CBaseEntity* ent = CBaseEntity::Create("doom_cacoball", bodyPos, angles, true, edict());
		EMIT_SOUND_DYN(ent->edict(), CHAN_BODY, "doom/dsfirsht.wav", 1.0f, ATTN_NORM, 0, 100);
	}

	void Think()
	{
		DoomThink();
	}
};

class CLostSoul : public CDoomMonster
{
	void Spawn()
	{
		bodySpriteName = "skul";

		animInfo.push_back(AnimInfo(0, 1, 0.25f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 1, 0.25f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(2, 3, 0.25f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(2, 3, 0.25f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(4, 4, 0.125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(40, 45, 0.5f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(40, 45, 0.5f, false)); // ANIM_GIB		

		idleSounds.push_back("doom/dsdmact.wav");
		painSound = "doom/dsdmpain.wav";
		deathSounds.push_back("doom/dsfirxpl.wav");
		this->meleeSound = "doom/dssklatk.wav";

		this->hasMelee = false;
		this->hasRanged = true;
		this->canFly = true;
		this->fullBright = true;
		this->painChance = 1.0f;
		this->deathBoom = 1;

		m_displayName = ALLOC_STRING("Lost Soul");
		pev->health = 100;

		DoomSpawn();

		SetThink(&CLostSoul::Think);
		SetTouch(&CLostSoul::Touched);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	void RangeAttack(Vector aimDir)
	{
		Dash(aimDir.Normalize() * 20 * g_monster_scale, RANDOM_LONG(3, 24), 1.0f);
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, meleeSound, 1.0f, 0.5f, 0, 100);
	}

	void Touched(CBaseEntity* ent)
	{
		DoomTouched(ent);
	}

	void Think()
	{
		DoomThink();
	}
};

class CPainElemental : public CDoomMonster
{	
	void Spawn()
	{
		bodySpriteName = "pain";
		
		animInfo.push_back(AnimInfo(0, 0, 0.125f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 2, 0.25f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(3, 5, 0.25f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(3, 5, 0.25f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(6, 6, 0.125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(56, 61, 0.25f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(56, 61, 0.5f, false)); // ANIM_GIB		
		
		idleSounds.push_back("doom/dsdmact.wav");
		painSound = "doom/dspepain.wav";
		deathSounds.push_back("doom/dspedth.wav");
		alertSounds.push_back("doom/dspesit.wav");
		
		this->hasMelee = false;
		this->hasRanged = true;
		this->canFly = true;
		this->painChance = 0.5f;
		this->deathBoom = 3;
		this->hullModel = "models/doom/null_fat.mdl";
		this->largeHull = true;
		
		m_displayName = ALLOC_STRING("Pain Elemental");
		pev->health = 400;
		
		DoomSpawn();
		
		SetThink( &CPainElemental::Think );
		pev->nextthink = gpGlobals->time + 0.1;
	}
	
	void MeleeAttack(Vector aimDir)
	{
		Slash(aimDir, RANDOM_LONG(10, 60));
	}
	
	void DeathBoom()
	{		
		g_engfuncs.pfnMakeVectors(pev->angles);
		Vector forward = gpGlobals->v_forward;
		Vector right = gpGlobals->v_right;
		ShootSoul(forward, false);
		ShootSoul((forward + right).Normalize(), false);
		ShootSoul((forward - right).Normalize(), false);
	}
	
	int CountSouls()
	{
		int count = 0;
		CBaseEntity* ent = NULL;
		do {
			ent = UTIL_FindEntityByClassname(ent, "monster_lostsoul");
			if (ent)
			{
				CDoomMonster* mon = (CDoomMonster*)ent;
				if (!mon->superDormant && !mon->killPoints)
					count++;
			}
		} while (ent);
		return count;
	}
	
	void ShootSoul(Vector aimDir, bool atEnemy)
	{
		if (CountSouls() >= 21)
		{
			ALERT(at_console, "Too many lost souls in level. Aborting attack.\n");
			return;
		}
		Vector flatAim = Vector(aimDir.x, aimDir.y, 0).Normalize();
		Vector spawnPos = BodyPos() + flatAim*64;
		Vector angles = UTIL_VecToAngles(aimDir);
		
		CBaseEntity* soul = CBaseEntity::Create("monster_lostsoul", spawnPos, angles, true);
		//*soul->pev->owner = *edict();
		//DispatchSpawn(soul->edict());
		
		CDoomMonster* mon = (CDoomMonster*)soul;
		mon->killPoints = false;
		mon->Setup();
		mon->dormant = false;
		
		
		Vector soulDir = aimDir;
		if (atEnemy) 
		{
			Vector enemyPos = h_enemy.GetEntity()->pev->origin + h_enemy.GetEntity()->pev->view_ofs;
			soulDir = (enemyPos - soul->pev->origin).Normalize();
			mon->SetEnemy(h_enemy);
		}
		
		TraceResult tstuck;
		Vector bodyPos = mon->BodyPos();
		UTIL_TraceHull( bodyPos, bodyPos, dont_ignore_monsters, human_hull, soul->edict(), &tstuck );
		if (tstuck.fAllSolid == 1)
		{
			// blow up if got stuck/spawned inside something
			CBaseEntity* pHit = CBaseEntity::Instance( tstuck.pHit );
			if (pHit)
				pHit->TakeDamage(mon->pev, mon->pev, mon->dashDamage, DMG_BURN);
			mon->TakeDamage(mon->pev, mon->pev, mon->pev->health, 0);
		}
		
		mon->RangeAttack(soulDir);
	}
	
	void RangeAttack(Vector aimDir)
	{
		brighten = 8;
		ShootSoul(aimDir, true);
	}
	
	void Think()
	{
		DoomThink();
	}
};


class CBaron : public CDoomMonster
{
	void Spawn()
	{
		bodySpriteName = "boss";

		animInfo.push_back(AnimInfo(0, 1, 0.125f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 3, 0.25f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(4, 6, 0.25f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(4, 6, 0.25f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(7, 7, 0.125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(64, 68, 0.25f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(64, 68, 0.5f, false)); // ANIM_GIB		

		idleSounds.push_back("doom/dsdmact.wav");
		painSound = "doom/dsdmpain.wav";
		deathSounds.push_back("doom/dsbrsdth.wav");
		alertSounds.push_back("doom/dsbrssit.wav");
		meleeSound = "doom/dsclaw.wav";

		this->hasMelee = true;
		this->hasRanged = true;
		this->painChance = 0.20f;
		this->hullModel = "models/doom/null_tall.mdl";

		m_displayName = ALLOC_STRING("Baron of Hell");
		pev->health = 1000;

		DoomSpawn();

		SetThink(&CBaron::Think);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	void Precache() override {
		CDoomMonster::Precache();
		UTIL_PrecacheOther("doom_baronball");
	}

	void MeleeAttack(Vector aimDir)
	{
		if (Slash(aimDir, RANDOM_LONG(10, 80)))
			EMIT_SOUND_DYN(edict(), CHAN_WEAPON, meleeSound, 1.0f, 0.5f, 0, 100);
	}

	void RangeAttack(Vector aimDir)
	{
		brighten = 8;

		Vector bodyPos = BodyPos();
		Vector angles = UTIL_VecToAngles(aimDir);

		CBaseEntity* ent = CBaseEntity::Create("doom_baronball", bodyPos, angles, true, edict());
		EMIT_SOUND_DYN(ent->edict(), CHAN_BODY, "doom/dsfirsht.wav", 1.0f, ATTN_NORM, 0, 100);
	}

	void Think()
	{
		DoomThink();
	}
};

class CHellKnight : public CDoomMonster
{
	void Spawn()
	{
		bodySpriteName = "bos2";

		animInfo.push_back(AnimInfo(0, 1, 0.125f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 3, 0.25f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(4, 6, 0.25f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(4, 6, 0.25f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(7, 7, 0.125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(64, 68, 0.25f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(64, 68, 0.5f, false)); // ANIM_GIB		

		idleSounds.push_back("doom/dsdmact.wav");
		painSound = "doom/dsdmpain.wav";
		deathSounds.push_back("doom/dskntdth.wav");
		alertSounds.push_back("doom/dskntsit.wav");
		meleeSound = "doom/dsclaw.wav";

		this->hasMelee = true;
		this->hasRanged = true;
		this->painChance = 0.50f;
		this->hullModel = "models/doom/null_tall.mdl";

		m_displayName = ALLOC_STRING("Hell Knight");
		pev->health = 500;

		DoomSpawn();

		SetThink(&CHellKnight::Think);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	void Precache() override {
		CDoomMonster::Precache();
		UTIL_PrecacheOther("doom_baronball");
	}

	void MeleeAttack(Vector aimDir)
	{
		if (Slash(aimDir, RANDOM_LONG(10, 80)))
			EMIT_SOUND_DYN(edict(), CHAN_WEAPON, meleeSound, 1.0f, 0.5f, 0, 100);
	}

	void RangeAttack(Vector aimDir)
	{
		brighten = 8;

		Vector bodyPos = BodyPos();
		Vector angles = UTIL_VecToAngles(aimDir);

		CBaseEntity* ent = CBaseEntity::Create("doom_baronball", bodyPos, angles, true, edict());
		EMIT_SOUND_DYN(ent->edict(), CHAN_BODY, "doom/dsfirsht.wav", 1.0f, ATTN_NORM, 0, 100);
	}

	void Think()
	{
		DoomThink();
	}
};

class CArchVile : public CDoomMonster
{
	EHANDLE flame;

	void Spawn()
	{
		bodySpriteName = "vile";

		animInfo.push_back(AnimInfo(0, 1, 0.125f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 5, 0.5f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(17, 19, 0.25f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(6, 15, 0.25f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(16, 16, 0.125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(160, 168, 0.375f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(160, 168, 0.375f, false)); // ANIM_GIB		

		animInfo[ANIM_ATTACK2].attackFrames.resize(0);
		animInfo[ANIM_ATTACK2].attackFrames.push_back(8);
		animInfo[ANIM_ATTACK2].frameIndices.push_back(15);
		animInfo[ANIM_ATTACK2].frameIndices.push_back(15);

		idleSounds.push_back("doom/dsvilact.wav");
		painSound = "doom/dsvipain.wav";
		deathSounds.push_back("doom/dsvildth.wav");
		alertSounds.push_back("doom/dsvilsit.wav");
		meleeSound = "doom/dsvilatk.wav";

		this->hasMelee = false;
		this->hasRanged = true;
		this->painChance = 0.04f;
		this->walkSpeed = 15.0f;
		this->canRevive = true;
		this->hullModel = "models/doom/null_tall.mdl";

		m_displayName = ALLOC_STRING("Arche-vile");
		pev->health = 700;

		DoomSpawn();

		SetThink(&CArchVile::Think);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	void Precache() override {
		CDoomMonster::Precache();
		UTIL_PrecacheOther("doom_vilefire");
	}

	void CastFire()
	{
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, meleeSound, 1.0f, 0.5f, 0, 100);

		CBaseEntity* enemy = h_enemy;

		if (!enemy)
			return;

		brighten = 46;

		CBaseEntity* fire = CBaseEntity::Create("doom_vilefire", enemy->pev->origin, g_vecZero, false, edict());
		CDoomProjectile* ball = (CDoomProjectile*)fire;
		ball->h_followEnt = this;
		ball->h_aimEnt = h_enemy;

		DispatchSpawn(fire->edict());
		fire->pev->solid = SOLID_NOT;
		fire->pev->movetype = MOVETYPE_NONE;
		fire->pev->rendermode = kRenderTransAdd;
		fire->pev->rendermode = kRenderTransTexture;
		fire->pev->renderamt = 180;

		EMIT_SOUND_DYN(fire->edict(), CHAN_BODY, "doom/dsflame.wav", 1.0f, ATTN_NORM, 0, 100);

		flame = fire;
	}

	void RangeAttackStart()
	{
		CastFire();
	}

	void RangeAttack(Vector aimDir)
	{
		if (!flame.GetEntity())
			return;

		CDoomProjectile* ball = (CDoomProjectile*)flame.GetEntity();
		ball->Touch(h_enemy);
		//ball.Remove();
	}

	void Think()
	{
		DoomThink();
	}
};


class CRevenant : public CDoomMonster
{
	void Spawn()
	{
		bodySpriteName = "skel";

		animInfo.push_back(AnimInfo(0, 1, 0.125f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 5, 0.5f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(6, 8, 1.0f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(9, 10, 0.5f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(11, 11, 0.125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(96, 100, 0.25f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(96, 100, 0.5f, false)); // ANIM_GIB		

		animInfo[ANIM_ATTACK].frameIndices.push_back(8);
		animInfo[ANIM_ATTACK].frameIndices.push_back(8);
		animInfo[ANIM_ATTACK].frameIndices.insert(animInfo[ANIM_ATTACK].frameIndices.begin() + 1, 7);
		animInfo[ANIM_ATTACK].frameIndices.insert(animInfo[ANIM_ATTACK].frameIndices.begin() + 1, 7);
		animInfo[ANIM_ATTACK].frameIndices.insert(animInfo[ANIM_ATTACK].frameIndices.begin(), 6);
		animInfo[ANIM_ATTACK].frameIndices.insert(animInfo[ANIM_ATTACK].frameIndices.begin(), 6);
		animInfo[ANIM_ATTACK].attackFrames.resize(0);
		animInfo[ANIM_ATTACK].attackFrames.push_back(6);

		animInfo[ANIM_ATTACK2].frameIndices.push_back(10);
		animInfo[ANIM_ATTACK2].frameIndices.push_back(10);
		animInfo[ANIM_ATTACK2].frameIndices.insert(animInfo[ANIM_ATTACK2].frameIndices.begin(), 9);
		animInfo[ANIM_ATTACK2].frameIndices.insert(animInfo[ANIM_ATTACK2].frameIndices.begin(), 9);
		animInfo[ANIM_ATTACK2].attackFrames.resize(0);
		animInfo[ANIM_ATTACK2].attackFrames.push_back(3);

		idleSounds.push_back("doom/dsskeact.wav");
		painSound = "doom/dspopain.wav";
		deathSounds.push_back("doom/dsskedth.wav");
		alertSounds.push_back("doom/dsskesit.wav");
		meleeSound = "doom/dsskepch.wav";
		meleeWindupSound = "doom/dsskeswg.wav";

		this->hasMelee = true;
		this->hasRanged = true;
		this->painChance = 0.39f;
		this->walkSpeed = 10.0f;
		this->rangeWhenMeleeFails = false;
		this->hullModel = "models/doom/null_tall.mdl";

		m_displayName = ALLOC_STRING("Revenant");
		pev->health = 300;

		DoomSpawn();

		SetThink(&CRevenant::Think);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	void Precache() override {
		CDoomMonster::Precache();
		UTIL_PrecacheOther("doom_revball");
	}

	void RangeAttackStart()
	{
		brighten = 8;
	}

	void MeleeAttack(Vector aimDir)
	{
		if (Slash(aimDir, RANDOM_LONG(8, 64)))
			EMIT_SOUND_DYN(edict(), CHAN_WEAPON, meleeSound, 1.0f, 0.5f, 0, 100);
	}

	void RangeAttack(Vector aimDir)
	{
		Vector bodyPos = BodyPos();
		Vector angles = UTIL_VecToAngles(aimDir);

		CBaseEntity* fireball = CBaseEntity::Create("doom_revball", bodyPos, angles, true, edict());

		CDoomProjectile* ball = (CDoomProjectile*)fireball;
		ball->h_followEnt = h_enemy;

		EMIT_SOUND_DYN(fireball->edict(), CHAN_BODY, "doom/dsskeatk.wav", 1.0f, ATTN_NORM, 0, 100);
	}

	void Think()
	{
		DoomThink();
	}
};

class CMancubus : public CDoomMonster
{
	int rangeCombo = 0;

	void Spawn()
	{
		bodySpriteName = "fatt";

		animInfo.push_back(AnimInfo(0, 1, 0.125f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 5, 0.25f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(6, 8, 0.375f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(6, 8, 0.375f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(9, 9, 0.125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(80, 89, 0.25f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(80, 89, 0.5f, false)); // ANIM_GIB		


		animInfo[ANIM_ATTACK].frameIndices.insert(animInfo[ANIM_ATTACK].frameIndices.begin(), 6);
		animInfo[ANIM_ATTACK].frameIndices.insert(animInfo[ANIM_ATTACK].frameIndices.begin(), 6);
		animInfo[ANIM_ATTACK].frameIndices.insert(animInfo[ANIM_ATTACK].frameIndices.begin(), 6);
		animInfo[ANIM_ATTACK].attackFrames.resize(0);
		animInfo[ANIM_ATTACK].attackFrames.push_back(4);
		animInfo[ANIM_ATTACK2] = animInfo[ANIM_ATTACK];
		this->constantAttackLoopFrame = 2;

		idleSounds.push_back("doom/dsposact.wav");
		painSound = "doom/dsmnpain.wav";
		deathSounds.push_back("doom/dsmandth.wav");
		alertSounds.push_back("doom/dsmansit.wav");

		this->hasMelee = false;
		this->hasRanged = true;
		this->painChance = 0.31f;
		this->walkSpeed = 8.0f;
		this->constantAttack = true;
		this->constantAttackMax = 3;
		this->hullModel = "models/doom/null_fat.mdl";
		this->largeHull = true;

		m_displayName = ALLOC_STRING("Mancubus");
		pev->health = 600;

		DoomSpawn();

		SetThink(&CMancubus::Think);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	void Precache() override {
		CDoomMonster::Precache();
		UTIL_PrecacheOther("doom_mancuball");
	}

	void ShootFireball(Vector origin, Vector addangles)
	{
		Vector enemyPos = h_enemy.GetEntity()->pev->origin + h_enemy.GetEntity()->pev->view_ofs;
		Vector aimDir = (enemyPos - origin).Normalize();

		Vector angles = UTIL_VecToAngles(aimDir);
		angles = angles + addangles;

		CBaseEntity* ent = CBaseEntity::Create("doom_mancuball", origin, angles, true, edict());
		EMIT_SOUND_DYN(ent->edict(), CHAN_BODY, "doom/dsfirsht.wav", 1.0f, ATTN_NORM, 0, 100);
	}

	void RangeAttack(Vector aimDir)
	{
		brighten = 8;
		g_engfuncs.pfnMakeVectors(pev->angles);

		Vector left = BodyPos() - gpGlobals->v_right * 32;
		Vector right = BodyPos() + gpGlobals->v_right * 32;
		if (rangeCombo == 0)
		{
			ShootFireball(right, Vector(0, 0, 0));
			ShootFireball(left, Vector(0, 15, 0));
		}
		else if (rangeCombo == 1)
		{
			ShootFireball(right, Vector(0, -15, 0));
			ShootFireball(left, Vector(0, 0, 0));
		}
		else
		{
			ShootFireball(right, Vector(0, -5, 0));
			ShootFireball(left, Vector(0, 5, 0));
		}

		if (++rangeCombo >= 3)
			rangeCombo = 0;
	}

	void Think()
	{
		DoomThink();
	}
};

class CArachnotron : public CDoomMonster
{
	void Spawn()
	{
		bodySpriteName = "bspi";

		animInfo.push_back(AnimInfo(0, 1, 0.125f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 5, 0.25f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(6, 7, 1.0f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(6, 7, 1.0f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(8, 8, 0.125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(72, 78, 0.25f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(72, 78, 0.5f, false)); // ANIM_GIB		


		animInfo[ANIM_ATTACK].frameIndices.insert(animInfo[ANIM_ATTACK].frameIndices.begin(), 6);
		animInfo[ANIM_ATTACK].frameIndices.insert(animInfo[ANIM_ATTACK].frameIndices.begin(), 6);
		animInfo[ANIM_ATTACK].frameIndices.insert(animInfo[ANIM_ATTACK].frameIndices.begin(), 6);
		animInfo[ANIM_ATTACK].frameIndices.insert(animInfo[ANIM_ATTACK].frameIndices.begin(), 6);
		animInfo[ANIM_ATTACK].frameIndices.push_back(7);
		animInfo[ANIM_ATTACK].attackFrames.resize(0);
		animInfo[ANIM_ATTACK].attackFrames.push_back(4);
		animInfo[ANIM_ATTACK2] = animInfo[ANIM_ATTACK];
		this->constantAttackLoopFrame = 3;

		idleSounds.push_back("doom/dsbspact.wav");
		painSound = "doom/dsdmpain.wav";
		deathSounds.push_back("doom/dsbspdth.wav");
		alertSounds.push_back("doom/dsbspsit.wav");
		walkSound = "doom/dsbspwlk.wav";

		this->hasMelee = false;
		this->hasRanged = true;
		this->painChance = 0.50f;
		this->walkSpeed = 12.0f;
		this->constantAttack = true;
		this->largeHull = true;
		this->hullModel = "models/doom/null_spider.mdl";

		m_displayName = ALLOC_STRING("Arachnotron");
		pev->health = 500;

		DoomSpawn();

		SetThink(&CArachnotron::Think);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	void Precache() override {
		CDoomMonster::Precache();
		UTIL_PrecacheOther("doom_spiderball");
	}

	void RangeAttack(Vector aimDir)
	{
		brighten = 8;

		Vector bodyPos = BodyPos();
		Vector angles = UTIL_VecToAngles(aimDir);

		CBaseEntity* ball =CBaseEntity::Create("doom_spiderball", bodyPos, angles, true, edict());

		EMIT_SOUND_DYN(ball->edict(), CHAN_BODY, "doom/dsplasma.wav", 1.0f, ATTN_NORM, 0, 100);
	}

	void Think()
	{
		DoomThink();
	}
};

class CCyberdemon : public CDoomMonster
{
	void Spawn()
	{
		bodySpriteName = "cybr";

		animInfo.push_back(AnimInfo(0, 1, 0.125f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 3, 0.25f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(4, 5, 0.25f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(4, 5, 0.25f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(6, 6, 0.125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(56, 64, 0.25f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(56, 64, 0.5f, false)); // ANIM_GIB		

		animInfo[ANIM_ATTACK].attackFrames.resize(0);
		animInfo[ANIM_ATTACK].frameIndices.push_back(4);
		animInfo[ANIM_ATTACK].attackFrames.push_back(1);
		animInfo[ANIM_ATTACK2] = animInfo[ANIM_ATTACK];

		idleSounds.push_back("doom/dsdmact.wav");
		painSound = "doom/dsdmpain.wav";
		deathSounds.push_back("doom/dscybdth.wav");
		alertSounds.push_back("doom/dscybsit.wav");
		meleeSound = "doom/dsclaw.wav";
		walkSound = "doom/dshoof.wav";
		hullModel = "models/doom/null_large.mdl";

		this->hasMelee = false;
		this->hasRanged = true;
		this->painChance = 0.08f;
		this->walkSpeed = 16.0f;
		this->constantAttack = true;
		this->constantAttackMax = 3;
		this->dmgImmunity = DMG_BLAST;
		this->largeHull = true;
		this->minRangeAttackDelay = 0.5f;
		this->maxRangeAttackDelay = 1.5f;

		m_displayName = ALLOC_STRING("Cyberdemon");
		pev->health = 4000;

		DoomSpawn();

		SetThink(&CCyberdemon::Think);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	void Precache() override {
		CDoomMonster::Precache();
		UTIL_PrecacheOther("doom_rocket");
	}

	void RangeAttack(Vector aimDir)
	{
		brighten = 8;

		Vector bodyPos = BodyPos();
		Vector angles = UTIL_VecToAngles(aimDir);

		CBaseEntity* ball = CBaseEntity::Create("doom_rocket", bodyPos, angles, true, edict());

		EMIT_SOUND_DYN(ball->edict(), CHAN_BODY, "doom/dsrlaunc.wav", 1.0f, ATTN_NORM, 0, 100);
	}

	void Think()
	{
		DoomThink();
	}
};

class CSpiderDemon : public CDoomMonster
{
	void Spawn()
	{
		bodySpriteName = "spid";

		animInfo.push_back(AnimInfo(0, 1, 0.125f, true)); // ANIM_IDLE
		animInfo.push_back(AnimInfo(0, 5, 0.25f, true)); // ANIM_MOVE
		animInfo.push_back(AnimInfo(6, 7, 0.5f, true)); // ANIM_ATTACK
		animInfo.push_back(AnimInfo(6, 7, 0.5f, true)); // ANIM_ATTACK2
		animInfo.push_back(AnimInfo(8, 8, 0.125f, true)); // ANIM_PAIN
		animInfo.push_back(AnimInfo(72, 81, 0.18f, false)); // ANIM_DEAD
		animInfo.push_back(AnimInfo(72, 81, 0.18f, false)); // ANIM_GIB		

		animInfo[ANIM_ATTACK].attackFrames.resize(0);
		animInfo[ANIM_ATTACK].attackFrames.push_back(0);
		animInfo[ANIM_ATTACK].attackFrames.push_back(1);
		animInfo[ANIM_ATTACK2] = animInfo[ANIM_ATTACK];

		animInfo[ANIM_DEAD].frameIndices.insert(animInfo[ANIM_DEAD].frameIndices.begin() + 1, 73);
		animInfo[ANIM_DEAD].frameIndices.insert(animInfo[ANIM_DEAD].frameIndices.begin(), 72);
		animInfo[ANIM_GIB].frameIndices = animInfo[ANIM_DEAD].frameIndices;

		idleSounds.push_back("doom/dsdmact.wav");
		painSound = "doom/dsdmpain.wav";
		deathSounds.push_back("doom/dsspidth.wav");
		alertSounds.push_back("doom/dsspisit.wav");
		shootSound = "doom/dsshotgn.wav";
		walkSound = "doom/dsmetal.wav";

		this->hasMelee = false;
		this->hasRanged = true;
		this->painChance = 0.16f;
		this->walkSpeed = 12.0f;
		this->constantAttack = true;
		this->largeHull = true;
		this->hullModel = "models/doom/null_huge.mdl";
		this->dmgImmunity = DMG_BLAST;

		m_displayName = ALLOC_STRING("Spiderdemon");
		pev->health = 3000;

		DoomSpawn();

		SetThink(&CSpiderDemon::Think);
		pev->nextthink = gpGlobals->time + 0.1;
	}

	void RangeAttack(Vector aimDir)
	{
		EMIT_SOUND_DYN(edict(), CHAN_WEAPON, shootSound, 1.0f, 0.5f, 0, 100);

		ShootBullet(aimDir, 22.0f, RANDOM_LONG(3, 15));
		ShootBullet(aimDir, 22.0f, RANDOM_LONG(3, 15), false);
		ShootBullet(aimDir, 22.0f, RANDOM_LONG(3, 15), false);
	}

	void Think()
	{
		DoomThink();
	}
};

LINK_ENTITY_TO_CLASS(monster_imp, CImp)
LINK_ENTITY_TO_CLASS(monster_zombieman, CZombieMan)
LINK_ENTITY_TO_CLASS(monster_cacodemon, CCacodemon)
LINK_ENTITY_TO_CLASS(monster_demon, CDemon)
LINK_ENTITY_TO_CLASS(monster_cyberdemon, CCyberdemon)
LINK_ENTITY_TO_CLASS(monster_hellknight, CHellKnight)
LINK_ENTITY_TO_CLASS(monster_hwdude, CHwDude)
LINK_ENTITY_TO_CLASS(monster_lostsoul, CLostSoul)
LINK_ENTITY_TO_CLASS(monster_painelemental, CPainElemental)
LINK_ENTITY_TO_CLASS(monster_baron, CBaron)
LINK_ENTITY_TO_CLASS(monster_mancubus, CMancubus)
LINK_ENTITY_TO_CLASS(monster_revenant, CRevenant)
LINK_ENTITY_TO_CLASS(monster_archvile, CArchVile)
LINK_ENTITY_TO_CLASS(monster_shotgunguy, CShotgunGuy)
LINK_ENTITY_TO_CLASS(monster_arachnotron, CArachnotron)
LINK_ENTITY_TO_CLASS(monster_spiderdemon, CSpiderDemon)