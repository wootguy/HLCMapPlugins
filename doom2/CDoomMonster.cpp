#include "extdll.h"
#include "util.h"
#include "CDoomMonster.h"
#include "CBasePlayer.h"
#include "CBasePlayerWeapon.h"
#include "CSoundEnt.h"
#include "doom_utils.h"
#include "doom2.h"
#include "te_effects.h"
#include "weapons.h"
#include "monsters.h"

using namespace std;

vector<string> light_suffix = {"_L3", "_L2", "_L1", "_L0"};

float g_monster_scale = 1.42857f; // 1 / 0.7
float g_monster_center_z = 34;
float g_monster_think_delay = 0.0572f;
uint32_t g_monster_idx = 0;

#define FL_MONSTER_DEAF 8
#define ATTN_DOOM 0.6f

AnimInfo::AnimInfo(int min, int max, float rate, bool loop)
{
	this->framerate = rate;
	this->looped = loop;
	this->attackFrames.push_back(max - min);

	for (int i = min; i <= max; i++)
		frameIndices.push_back(i);
}

int AnimInfo::getFrameIdx(int frameCounter, float oldCounter, bool& looped)
{
	int oldIdx = int(oldCounter * framerate) % frameIndices.size();
	int newIdx = int(frameCounter * framerate) % frameIndices.size();
	looped = oldIdx > newIdx || frameIndices.size() == 1;
	return newIdx;
}

bool AnimInfo::isAttackFrame(int idx)
{
	for (int i = 0; i < attackFrames.size(); i++)
	{
		if (attackFrames[i] == idx)
			return true;
	}
	return false;
}

int AnimInfo::lastFrame()
{
	return frameIndices[frameIndices.size() - 1];
}



void CDoomMonster::KeyValue(KeyValueData* pkvd)
{
	if (FStrEq(pkvd->szKeyName, "spectre"))
	{
		isSpectre = atoi(pkvd->szValue) != 0;
		pkvd->fHandled = TRUE;
	}
	else if (FStrEq(pkvd->szKeyName, "doom_flags"))
	{
		isDeaf = (atoi(pkvd->szValue) & FL_MONSTER_DEAF) != 0;
		pkvd->fHandled = TRUE;
	}
	else {
		CBaseMonster::KeyValue(pkvd);
	}
}
	
void CDoomMonster::Precache()
{
	if (bodySpriteName) {
		bodySpriteHw = STRING(ALLOC_STRING(UTIL_VarArgs("sprites/doom/hw/%s.spr", bodySpriteName)));
		bodySpriteSw = STRING(ALLOC_STRING(UTIL_VarArgs("sprites/doom/sw/%s.spr", bodySpriteName)));
	}

	for (int i = 0; i < idleSounds.size(); i++)
	{
		PRECACHE_SOUND(idleSounds[i]);
	}
	for (int i = 0; i < deathSounds.size(); i++)
	{
		PRECACHE_SOUND(deathSounds[i]);
	}
	for (int i = 0; i < alertSounds.size(); i++)
		PRECACHE_SOUND(alertSounds[i]);
	if (meleeWindupSound)
		PRECACHE_SOUND(meleeWindupSound);
	
	if (meleeSound)
		PRECACHE_SOUND(meleeSound);
	if (shootSound)
		PRECACHE_SOUND(shootSound);
	if (painSound)
		PRECACHE_SOUND(painSound);
	if (walkSound)
		PRECACHE_SOUND(walkSound);
	if (bodySpriteHw)
		modelIndexHw = PRECACHE_MODEL(bodySpriteHw);
	if (bodySpriteSw)
		modelIndexSw = PRECACHE_MODEL(bodySpriteSw);
	if (hullModel)
		PRECACHE_MODEL(hullModel);

	PRECACHE_SOUND("doom/dsslop.wav");
}
	
void CDoomMonster::DelayAttack()
{
	nextRangeAttack = gpGlobals->time + RANDOM_FLOAT(minRangeAttackDelay, maxRangeAttackDelay);
}
	
void CDoomMonster::DoomSpawn()
{
	pev->flags |= FL_MONSTER;

	Precache();

	if (!g_map_init_done) {
		UTIL_Remove(this);
		return;
	}
}

void CDoomMonster::CreateRenderSprites() {
	if (m_hSprite.GetEntity())
		return;

	CBaseEntity* spr = CBaseEntity::Create("doom_sprite", pev->origin, g_vecZero, true);
	SET_MODEL(spr->edict(), bodySpriteHw);
	spr->pev->targetname = ALLOC_STRING(UTIL_VarArgs("m%ds", g_monster_idx++));
	spr->pev->movetype = MOVETYPE_FOLLOW;
	spr->pev->aiment = edict();

	if (bodySpriteSw) {
		CDoomSprite* dspr = (CDoomSprite*)spr;
		dspr->modelIndexSw = MODEL_INDEX(bodySpriteSw);
	}

	m_hSprite = spr;
}

void CDoomMonster::SetupHull()
{
	if (largeHull)
		UTIL_SetSize(pev, Vector(-32, -32, -7), Vector(32, 32, 72));
	else {
		//UTIL_SetSize(pev, Vector(-12, -12, -7), Vector(12, 12, 42));
		UTIL_SetSize(pev, Vector(-16, -16, -7), Vector(16, 16, 64));
	}
}
	
void CDoomMonster::Setup()
{
	if (!superDormant)
		return;
	pev->movetype = canFly ? MOVETYPE_FLY : MOVETYPE_STEP;
	pev->solid = SOLID_NOT;
		
	//SET_MODEL(edict(), "models/doom/null.mdl");
	SET_MODEL(edict(), hullModel);
	//UTIL_SetSize(pev, Vector(-16, -16, 0), Vector(16, 16, 72));
		
	m_bloodColor = BloodColorHuman();
	pev->scale = g_monster_scale;
	pev->takedamage = DAMAGE_YES;
	pev->flags |= FL_MONSTERCLIP;
		
	MonsterInit();
		
	UTIL_SetSize(pev, VEC_HUMAN_HULL_MIN, VEC_HUMAN_HULL_MAX);
	SetClassification(CLASS_ALIEN_MONSTER);
	SetActivity(ANIM_IDLE);
		
	pev->view_ofs = Vector(0,0,56);
		
	if (canFly || true)
		pev->origin.z -= 16;
		
	SetupHull();
	
	superDormant = false;
	pev->solid = SOLID_SLIDEBOX;
	if (!killPoints) // monster was spawned by something, create sprites now to avoid invisible monster bug
		CreateRenderSprites();
	pev->nextthink = gpGlobals->time + RANDOM_FLOAT(0.1f, 0.5f);
			
	lastDormantPos = pev->origin;
	dormantNode = getSoundNode(pev->origin);
}
	
int CDoomMonster::SetActivity(int act)
{
	if (act < 0 || act >= int(animInfo.size())) {
		ALERT(at_warning, "Bad activity: %d", act);
	}
	else
		currentAnim = animInfo[act];
		
	//ALERT(at_console, "ACT " + activity);
	animLoops = 0;
	oldFrameCounter = frameCounter = 0;
	activity = act;
		
	bool looped;
	int frameIdx = currentAnim.getFrameIdx(frameCounter, oldFrameCounter, looped);
	return currentAnim.frameIndices[frameIdx];
}
	
void CDoomMonster::Wakeup()
{
	if (!dormant)
		return;
	SetActivity(ANIM_MOVE);
	//ALERT(at_console, "IM AWAKE");
	if (alertSounds.size() > 0)
	{
		const char* snd = alertSounds[RANDOM_LONG(0, alertSounds.size()-1)];
		EMIT_SOUND_DYN(edict(), CHAN_BODY, snd, 1.0f, ATTN_DOOM, 0, 100);
	}
		
	dormant = false;
	nextIdleSound = gpGlobals->time + RANDOM_FLOAT(5.0f, 10.0f);
}

void CDoomMonster::Killed(entvars_t* pevAttacker, int iGib) {
	if (HasMemory(bits_MEMORY_KILLED)) {
		return;
	}

	CBaseMonster::Killed(pevAttacker, GIB_NEVER);
	pev->solid = SOLID_NOT; // immediately non-solid so other bullet tracers can pass thru
	pev->deadflag = DEAD_DYING;
	pev->health = 0;
	pev->frame = 0;
	pev->movetype = MOVETYPE_STEP;
	m_isFadingOut = true; // prevent pvs corpse removal

	SET_MODEL(edict(), bodySpriteHw);
	UTIL_Remove(m_hSprite);
	pev->renderamt = 255;
	pev->rendermode = 2;
	if (isSpectre)
	{
		pev->renderamt = 48;
	}

	bool gib = iGib == GIB_ALWAYS;
	SetActivity(gib ? ANIM_GIB : ANIM_DEAD);
	h_enemy = NULL;
	

	if (canFly && deathBoom == 0)
	{
		pev->velocity.z = -0.1f;
	}
	UTIL_SetSize(pev, Vector(-1, -1, -1), Vector(1, 1, 1));

	if (dropItem && dropItem[0])
	{
		Vector delta = (pevAttacker->origin - pev->origin).Normalize() * 32;
		CBaseEntity* item = CBaseEntity::Create(dropItem, pev->origin + Vector(0, 0, 8), g_vecZero);

		if (item) {
			item->pev->velocity = Vector(delta.x, delta.y, 350);
			item->pev->spawnflags |= SF_NORESPAWN;

			CBasePlayerWeapon* wep = item->GetWeaponPtr();
			if (wep) {
				if (string(dropItem) == "weapon_doom_shotgun") {
					wep->m_iDefaultAmmo = 4;
				}
				if (string(dropItem) == "weapon_doom_chaingun") {
					wep->m_iDefaultAmmo = 10;
				}
			}
		}
		else {
			ALERT(at_error, "Invalid drop item %s\n", dropItem);
		}
	}

	if (killPoints)
		g_kills += 1;

	const char* snd = deathSounds[RANDOM_LONG(0, deathSounds.size() - 1)];
	bool canGib = animInfo[ANIM_DEAD].frameIndices[0] != animInfo[ANIM_GIB].frameIndices[0];
	if (gib && canGib)
		snd = "doom/dsslop.wav";
	EMIT_SOUND_DYN(edict(), CHAN_ITEM, snd, 1.0f, ATTN_DOOM, 0, 100);

	DoomThink();
}

int CDoomMonster::TakeDamage( entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType )
{
	if ((bitsDamageType & DMG_BLAST) != 0 || flDamage > 100) {
		bitsDamageType |= DMG_ALWAYSGIB;
	}
	else {
		bitsDamageType &= ~DMG_ALWAYSGIB;
	}

	int ret = CBaseMonster::TakeDamage(pevInflictor, pevAttacker, flDamage, bitsDamageType);
	
	if (IsAlive()) {
		CBaseEntity* attacker = CBaseEntity::Instance(pevAttacker);
		SetEnemy(attacker);
	}
		
	return 0;
}

void CDoomMonster::PainSound(void) {
	if (RANDOM_FLOAT(0, 1) <= painChance) {
		SetActivity(ANIM_PAIN);
		//DelayAttack();
		EMIT_SOUND_DYN(edict(), CHAN_ITEM, painSound, 1.0f, ATTN_DOOM, 0, 100);
	}
}

void CDoomMonster::SetEnemy(CBaseEntity* ent)
{
	if (ent  == NULL  || !ent->IsAlive() || (ent->pev->flags & FL_NOTARGET) != 0 || ent->entindex() == entindex())
		return;
	if (string(STRING(ent->pev->classname)) == "monster_archvile")
		return; // never target these guys, so says the wiki
			
	// only switch targets after chasing current one for a while
	if (!h_enemy.GetEntity() || lastEnemy + 10.0f < gpGlobals->time)
	{
		if (h_enemy.GetEntity() && h_enemy.GetEntity()->IsPlayer())
			oldEnemy = h_enemy;
		//if (oldEnemy)
		//	ALERT(at_console, "I will remember to attack " + oldEnemy.GetEntity()->pev->netname);
		h_enemy = EHANDLE(ent->edict());
		lastEnemy = gpGlobals->time;
		DelayAttack();
		Wakeup();
	}
}
	
void CDoomMonster::ClearEnemy()
{
	h_enemy = oldEnemy;
	//if (h_enemy)
	//	ALERT(at_console, "I will go back to attacking " + h_enemy.GetEntity()->pev->classname);
	oldEnemy = NULL;
	if (!h_enemy.GetEntity())
		Sleep();
}
	
void CDoomMonster::Sleep()
{
	if (dormant)
		return;
			
	h_enemy = NULL;
	dormant = true;
	SetActivity(ANIM_IDLE);
}
	
Vector CDoomMonster::BodyPos()
{
	return pev->origin + Vector(0,0,g_monster_center_z);
}
	
void CDoomMonster::RangeAttack(Vector aimDir)
{
	ALERT(at_error, "Range attack not implemented!");
}
	
void CDoomMonster::MeleeAttack(Vector aimDir)
{
	ALERT(at_error, "Melee attack not implemented!");
}
	
void CDoomMonster::ShootBullet(Vector dir, float spread, float damage, bool flash)
{
	Vector vecSrc = BodyPos() + gunPos;
	float range = 16384;
		
	if (flash)
	{
		int flash_size = 20;
		int flash_life = 1;
		int flash_decay = 0;
		RGB flash_color = RGB(255, 160, 64);
		UTIL_DLight(vecSrc, flash_size, flash_color, flash_life, flash_decay);
		brighten = 4;
	}
		
	Vector vecAiming = spreadDir(dir.Normalize(), spread, SPREAD_GAUSSIAN);
	
	//te_beampoints(vecSrc, vecSrc + dir.Normalize()*range);
	
	HitScan(this, vecSrc, dir, spread, damage);
}

bool CDoomMonster::Slash(Vector dir, float damage)
{
	Vector bodyPos = BodyPos() + gunPos;
	TraceResult tr;
	Vector attackDir = dir.Normalize();
	UTIL_TraceHull( bodyPos, bodyPos + attackDir*meleeRange, dont_ignore_monsters, head_hull, edict(), &tr );
	CBaseEntity* phit = CBaseEntity::Instance( tr.pHit );
	//te_beampoints(bodyPos, bodyPos + delta.Normalize()*meleeRange);
		
	if (phit )
	{
		ClearMultiDamage();
		phit->TraceAttack(pev, damage, attackDir, &tr, DMG_SLASH);
		ApplyMultiDamage(pev, pev);
		knockBack(phit, attackDir*(100+damage));
		return true;
	}
	return false;
}
	
void CDoomMonster::Dash(Vector velocity, float damage, float timeout)
{
	dashVel = velocity;
	dashing = true;
	dashDamage = damage;
	dashTimeout = gpGlobals->time + timeout;
}
	
bool CDoomMonster::isAttacking()
{
	return activity == ANIM_ATTACK || activity == ANIM_ATTACK2;
}

void CDoomMonster::Revive()
{
	EMIT_SOUND_DYN(edict(), CHAN_ITEM, "doom/dsslop.wav", 1.0f, ATTN_DOOM, 0, 100);
	isCorpse = false;
		
	AnimInfo reverseAnim;
	reverseAnim.framerate = currentAnim.framerate;
	reverseAnim.looped = currentAnim.looped;		
	for (int i = currentAnim.frameIndices.size()-1; i >= 0; i--)
		reverseAnim.frameIndices.push_back(currentAnim.frameIndices[i]);
		
	SetActivity(activity);
	currentAnim = reverseAnim;
	isBeingRevived = true;
	m_afMemory = MEMORY_CLEAR;
	DoomThink();
}
	
bool CDoomMonster::ReviveNearbyDemon()
{
	CBaseEntity* ent = NULL;
	do {
		ent = UTIL_FindEntityInSphere(ent, pev->origin, revive_range); 
		if (ent && !ent->IsAlive() && string(STRING(ent->pev->classname)).find("monster_") == 0)
		{
			string cname = STRING(ent->pev->classname);
			if (cname == "monster_cyberdemon" || cname == "monster_spiderdemon" || cname == "monster_lostsoul" ||
				cname == "monster_painelemental" || cname == "monster_archvile")
			{
				continue;
			}
				
			if ((ent->pev->origin - pev->origin).Length() < 80)
				continue;
				
			CDoomMonster* mon = (CDoomMonster*)ent;
			if (mon && !mon->isBeingRevived)
			{
				//ALERT(at_console, "NEARBY MON: " + mon->pev->classname);
				mon->Revive();
				mon->h_enemy = h_enemy;
				return true;
			}
		}
	} while (ent);
	return false;
}
	
void CDoomMonster::DoomThink()
{
	if (superDormant)
		return;
	//te_beampoints(pev->origin, pev->origin + Vector(0,0,72));
		
	pev->oldorigin = pev->origin;
		
	if (isCorpse)
	{
		UTIL_Remove(this);			
		return;
	}
		
	RGB light_level = RGB(255, 255, 255);
	if (!fullBright)
	{
		light_level = g_bsp.get_lighting(pev->origin).ApplyGamma();

		if (brighten > 0)
		{
			brighten--;
			light_level.r = V_min(255, light_level.r + brighten*64);
			light_level.g = V_min(255, light_level.g + brighten*64);
			light_level.b = V_min(255, light_level.b + brighten*64);
		}
		//light_level = 255;
	}
	Vector lightColor = light_level.ToVector();
		
	//pev->rendercolor = lightColor;
	//ALERT(at_console, "LIGHT " + g_engfuncs.pfnGetEntityIllum(edict()) + " " + pev->light_level);
		
	//pev->velocity.z += -0.001f; // prevents floating && fixes fireballs not getting Touched by monsters that don't move
	Vector pos = pev->origin;
	UTIL_SetOrigin(pev, pos + Vector(0,0,1));
	UTIL_SetOrigin(pev, pos);
		
	//ALERT(at_console, "CURENT ANIM: " + currentAnim.frameIndices[0] + " " + currentAnim.lastFrame());
		
	frameCounter += 1;
	bool looped = false;
	int frameIdx = currentAnim.getFrameIdx(frameCounter, oldFrameCounter, looped);
	int frame = currentAnim.frameIndices[frameIdx];

	if (looped)
	{
		if (isAttacking() && constantAttack)
		{
			// increment frame until we get to the one we want
			int failsafe = 256;
			while (frameIdx != constantAttackLoopFrame)
			{
				frameCounter += 1;
				frameIdx = currentAnim.getFrameIdx(frameCounter, oldFrameCounter, looped);
				frame = currentAnim.frameIndices[frameIdx];
				failsafe -= 1;
				if (failsafe <= 0)
				{
					ALERT(at_console, "FAILED TO FIND CONSTANT ATTACK FRAME");
					break;
				}
			}
			looped = true;
		}
		animLoops += 1;
	}
		
	if (activity == ANIM_MOVE && walkSound && walkSound[0] && nextWalkSound < gpGlobals->time)
	{
		nextWalkSound = gpGlobals->time + walkSoundFreq;
		EMIT_SOUND_DYN(edict(), CHAN_ITEM, walkSound, 1.0f, ATTN_DOOM, 0, 100);
	}
		
	//ALERT(at_console, "FRAME " + frame + " " + currentAnim.minFrame + " " + currentAnim.maxFrame);
	
	g_engfuncs.pfnMakeVectors(pev->angles);
	Vector forward = gpGlobals->v_forward;
	Vector right = gpGlobals->v_right;
	forward.z = 0;
	right.z = 0;
	forward = forward.Normalize();
	right = right.Normalize();
		
	if (activity == ANIM_PAIN && animLoops > 3)
		frame = SetActivity(ANIM_MOVE);
		
	if (pev->health > 0)
		pev->deadflag = 0;
		
	if (!IsAlive())
	{
		if (looped)
		{
			if (isBeingRevived)
			{
				SetActivity(ANIM_IDLE);
				pev->deadflag = DEAD_NO;
				pev->health = pev->max_health;
				isBeingRevived = false;
				pev->renderamt = 0;
				pev->solid = SOLID_SLIDEBOX;
				CreateRenderSprites();
				SetupHull();
				Sleep();
				if (killPoints)
					g_kills -= 1; // no longer counts as a kill
				if (canFly)
					pev->origin.z += 16;
				pev->nextthink = gpGlobals->time;
				return;
			}
			else
			{
				isCorpse = true;
				pev->frame = currentAnim.lastFrame();
				if (deathBoom > 0)
				{
					UTIL_Remove(this);
					return;
				}
				pev->nextthink = gpGlobals->time + deathRemoveDelay;
				return;
			}
		}
		else
		{
			pev->frame = frame;
			if (deathBoom > 0 && frameIdx == deathBoom && !didDeathBoom)
			{
				didDeathBoom = true;
				DeathBoom();
			}
		}
		pev->rendercolor = lightColor;
	}
		
	if (!dormant && IsAlive())
	{
		Vector bodyPos = BodyPos();
			
		if (dashing)
		{
			if (gpGlobals->time > dashTimeout)
			{
				dashing = false;
				frame = SetActivity(ANIM_MOVE);
			}
			else if (activity != ANIM_ATTACK)
				frame = SetActivity(ANIM_ATTACK);
		}
			
		if (canRevive && lastReviveCheck + reviveDelay < gpGlobals->time && !isAttacking())
		{
			lastReviveCheck = gpGlobals->time;
			if (RANDOM_LONG(0, 100) <= 50)
			{
				if (ReviveNearbyDemon())
					frame = SetActivity(ANIM_ATTACK);
			}
		}
			
		Vector eyePos = pev->origin + pev->view_ofs;
		bool lineOfSight = false;
		if (h_enemy.GetEntity())
		{
			CBaseEntity* enemy = h_enemy;
			TraceResult tr_sight;
			UTIL_TraceLine( eyePos, enemy->pev->origin, ignore_monsters, edict(), &tr_sight );
			lineOfSight = tr_sight.flFraction >= 1.0f;
		}
			
			
		if (activity == ANIM_MOVE || dashing)
		{
			int canWalk = 0;
				
			Vector verticalMove = Vector(0,0,0);					
			if (canFly)
			{
				if (h_enemy.GetEntity())
				{
					CBaseEntity* enemy = h_enemy;
						
					//ALERT(at_console, "HEIGHT DIFF: " + (enemy->pev->origin.z - bodyPos.z));
						
					float enemyZ = enemy->pev->origin.z + enemy->pev->view_ofs.z;
					if (enemyZ > bodyPos.z + g_monster_center_z || !lineOfSight)
						verticalMove = Vector(0,0,4);
					else if (enemyZ < bodyPos.z - g_monster_center_z)
						verticalMove = Vector(0,0,-4);
				}
						
				TraceResult tVert;
				UTIL_TraceHull( bodyPos, bodyPos + verticalMove*2, dont_ignore_monsters, human_hull, edict(), &tVert );
				if (tVert.flFraction < 1.0f)
				{
					verticalMove = Vector(0,0,0);
					//ALERT(at_console, "NOT OK TO MOVE VERT");
				}
				else
				{
					//ALERT(at_console, "OK TO MOVE VERT");
				}
					
				TraceResult tr;
				Vector moveVel = forward*walkSpeed + verticalMove;
				if (dashing)
					moveVel = dashVel;
				Vector targetPos = bodyPos + moveVel*g_monster_scale;
				UTIL_TraceHull( bodyPos, targetPos, dont_ignore_monsters, human_hull, edict(), &tr );
				if (tr.flFraction >= 1.0f && tr.fAllSolid == 0)
				{
					canWalk = 1;
					Vector flyPos = tr.vecEndPos + Vector(0,0,-g_monster_center_z);
					UTIL_SetOrigin(pev, flyPos);
				}
				else if (dashing)
				{
					dashing = false;
					frame = SetActivity(ANIM_MOVE);
					CBaseEntity* pHit = CBaseEntity::Instance( tr.pHit );
					if (pHit )
					{
						DelayAttack();
						Vector oldVel = pHit->pev->velocity;
						pHit->TakeDamage(pev, pev, dashDamage, DMG_BURN);
						pHit->pev->velocity = oldVel; // prevent vertical launching
							
						knockBack(pHit, dashVel.Normalize()*(100 + dashDamage));
					}
				}
			}
			else
			{
				canWalk = g_engfuncs.pfnWalkMove(edict(), pev->angles.y, walkSpeed*g_monster_scale, int(WALKMOVE_NORMAL));
			}
				
			if (canWalk != 1 && lastWallReflect + 0.2f < gpGlobals->time)
			{
				pev->angles.y += RANDOM_FLOAT(90, 270);
				lastWallReflect = gpGlobals->time;
				lastDirChange = gpGlobals->time;
			}
		}
			
		if (h_enemy && (h_enemy.GetEntity()->pev->flags & FL_NOTARGET) != 0)
			ClearEnemy();
		
		if (h_enemy.GetEntity() && !dashing)
		{
			CBaseEntity* enemy = h_enemy;
				
			Vector enemyPos = enemy->pev->origin + enemy->pev->view_ofs;
			if (!enemy->IsPlayer())
			{
				enemyPos.z += (enemy->pev->maxs.z - enemy->pev->mins.z)*0.5f;
			}
			Vector delta = enemyPos - bodyPos;
			if (activity == ANIM_MOVE && lastDirChange + 1.0f < gpGlobals->time)
			{
				float idealYaw = g_engfuncs.pfnVecToYaw(delta);
				pev->angles.y = idealYaw;
				if (RANDOM_LONG(0,3) <= 1) // zig zag towards target
				{
					if (RANDOM_LONG(0,1) == 0)
						pev->angles.y += 45;
					else
						pev->angles.y -= 45;
				}
					
				lastDirChange = gpGlobals->time;
			}
				
			bool inMeleeRange = hasMelee && delta.Length() < meleeRange;
				
			if (nextRangeAttack + 0.5f < gpGlobals->time)
				DelayAttack(); // don't attack immediately when enemy comes back into view
				
			if (isAttacking() || inMeleeRange || (nextRangeAttack < gpGlobals->time && hasRanged && lineOfSight))
			{
				pev->angles.y = g_engfuncs.pfnVecToYaw(delta);
				if (!isAttacking())
				{
					if (inMeleeRange)
					{
						if (meleeWindupSound && meleeWindupSound[0])
							EMIT_SOUND_DYN(edict(), CHAN_BODY, meleeWindupSound, 1.0f, ATTN_DOOM, 0, 100);
						frame = SetActivity(ANIM_ATTACK);
						MeleeAttackStart();
					}
					else
					{
						frame = SetActivity(ANIM_ATTACK2);
						RangeAttackStart();
					}
				}
				else
				{
					if ((currentAnim.isAttackFrame(frameIdx)) && oldFrameIdx != frameIdx)
					{
						if (inMeleeRange)
							MeleeAttack(delta);
						else if (hasRanged)			
						{
							//te_beampoints(bodyPos, enemyPos);		
							if (rangeWhenMeleeFails || activity == ANIM_ATTACK2)
								RangeAttack(delta);
						}
					}
						
					if (animLoops > 0)
					{
						if (inMeleeRange)
						{
							if (meleeWindupSound && meleeWindupSound[0])
								EMIT_SOUND_DYN(edict(), CHAN_BODY, meleeWindupSound, 1.0f, ATTN_DOOM, 0, 100);
							frame = SetActivity(ANIM_ATTACK);
						}
						else
						{
							bool keepAttacking = false;
							if (constantAttack && (animLoops < constantAttackMax || constantAttackMax == 0))
							{
								TraceResult tr;
								UTIL_TraceLine( eyePos, enemy->pev->origin, dont_ignore_monsters, edict(), &tr );
								CBaseEntity* pHit = CBaseEntity::Instance( tr.pHit );
								keepAttacking = pHit  && pHit->entindex() == enemy->entindex();
									
								// HACK
								keepAttacking = keepAttacking || string(STRING(pev->classname)) == "monster_cyberdemon";
							}
							if (!keepAttacking)
							{
								DelayAttack();
								frame = SetActivity(ANIM_MOVE);
							}
						}
					}
				}
			}
				
			if (!enemy->IsAlive())
				ClearEnemy();
		}
			
		if (nextIdleSound < gpGlobals->time)
		{
			const char* snd = idleSounds[RANDOM_LONG(0, idleSounds.size()-1)];
			EMIT_SOUND_DYN(edict(), CHAN_ITEM, snd, 1.0f, ATTN_DOOM, 0, 100);
			nextIdleSound = gpGlobals->time + RANDOM_FLOAT(5.0f, 10.0f);
		}
	}
		
	// target visible players
	if (IsAlive() && !h_enemy.GetEntity())
	{
		for (int i = 1; i <= gpGlobals->maxClients; i++) {
			CBasePlayer* plr = UTIL_PlayerByIndex(i);

			if (!plr || (m_pvsPlayers & PLRBIT(i)) == 0 || !plr->IsAlive())
				continue;

			CreateRenderSprites();

			Vector delta = pev->origin - plr->pev->origin;
			delta = delta.Normalize();
			if (DotProduct(forward, delta) < -0.3f && plr->FVisible(this, true))
			{
				PlayerState& state = getPlayerState(plr);
				bool visible = plr->pev->rendermode == 0 || state.lastAttack + 1.0f > gpGlobals->time;
				visible = visible && UTIL_PointContents(plr->pev->origin) != CONTENTS_SOLID;

				if (visible)
					SetEnemy(plr);
			}
		}
	}
		
	// react to sounds
	if (dormant)
	{
		int activeList = CSoundEnt::ActiveList();
		while (activeList > -1)
		{
			CSound* snd = CSoundEnt::SoundPointerForIndex(activeList);
			if (snd == NULL)
				break;
			bool gunshot = snd->m_iVolume > 500;
			float reactDist = 2000;
			CBaseEntity* owner = snd->m_hOwner;
				
			if (gunshot && owner && owner->IsPlayer())
			{
				Vector delta = snd->m_vecOrigin - pev->origin;
				if (delta.Length() < reactDist)
				{
					delta.z = 0;
					bool moved = (lastDormantPos - pev->origin).Length() > 1.0f;
					if (moved)
					{
						lastDormantPos = pev->origin;
						dormantNode = getSoundNode(lastDormantPos);
						//ALERT(at_console, "NEW DORMANT NODE");
					}
						
					if (dormantNode)
					{
						PlayerState& state = getPlayerState((CBasePlayer*)(owner));
						if (canHearSound(dormantNode, state.soundNode, owner->pev->origin, this))
						{
							if (isDeaf)
								g_engfuncs.pfnVecToAngles(delta.Normalize(), pev->angles);
							else
								SetEnemy(owner);
						}
					}
					else
						g_engfuncs.pfnVecToAngles(delta.Normalize(), pev->angles);
				}
			}
					
			activeList = snd->m_iNext;
		}
	}

	UTIL_MakeVectors(pev->angles);

	pev->rendermode = kRenderNormal;
	pev->renderamt = 255;
	

	if (isSpectre) {
		pev->rendermode = kRenderTransAlpha;
		pev->renderamt = 48;
		//lightColor = Vector(0, 0, 0); // doesn't work
	}

	if (m_hSprite) {
		CDoomSprite* spr = (CDoomSprite*)m_hSprite.GetEntity();
		spr->forwardDir = gpGlobals->v_forward;
		spr->rightDir = gpGlobals->v_right;
		spr->oriented = true;
		spr->pev->frame = frame;
		spr->pev->scale = pev->scale;
		spr->pev->rendercolor = lightColor;
		spr->pev->rendermode = pev->rendermode;
		spr->pev->renderamt = pev->renderamt;
	}
	
	oldFrameCounter = frameCounter;
	oldFrameIdx = frameIdx;
	pev->nextthink = gpGlobals->time + g_monster_think_delay;
	//pev->nextthink = gpGlobals->time + 0.02857;
}

void CDoomMonster::UpdateOnRemove(void) {
	UTIL_Remove(m_hSprite);
}

int CDoomMonster::AddToFullPack(struct entity_state_s* state, CBasePlayer* player) {
	if (modelIndexHw == state->modelindex && player->m_clientRenderer == CLIENT_RENDERER_SOFTWARE) {
		state->origin.z += 8;
		if (modelIndexSw) {
			state->modelindex = modelIndexSw;
		}
	}

	return 1;
}