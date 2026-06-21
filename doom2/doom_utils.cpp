#include "extdll.h"
#include "util.h"
#include "doom_utils.h"
#include "CBasePlayer.h"
#include "decals.h"
#include "weapons.h"
#include "te_effects.h"
#include "shake.h"

using namespace std;

Vector g_vecAttackDir;

// Will create a new state if the requested one does not exit
PlayerState& getPlayerState(CBasePlayer* plr)
{
	uint64_t id64 = plr->GetSteamID64();
	
	auto item = g_player_states.find(id64);

	if (item == g_player_states.end())
	{
		PlayerState state = PlayerState();
		g_player_states[id64] = state;
		item = g_player_states.find(id64);
	}
	return item->second;
}

TraceResult TraceLook(CBasePlayer* plr, float dist, bool bigHull)
{
	Vector vecSrc = plr->GetGunPosition();
	MAKE_VECTORS( plr->pev->v_angle ); // todo: monster angles
	
	TraceResult tr;
	Vector vecEnd = vecSrc + gpGlobals->v_forward * dist;
	if (bigHull)
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, plr->edict(), &tr );
	else
		UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, plr->edict(), &tr );
	return tr;
}

void delay_remove(EHANDLE ent)
{
	UTIL_Remove(ent);
}

vector<float> rotationMatrix(Vector axis, float angle)
{
	axis = axis.Normalize();
    float s = sin(angle);
    float c = cos(angle);
    float oc = 1.0 - c;
 
	vector<float> mat = {
		oc * axis.x * axis.x + c,          oc * axis.x * axis.y - axis.z * s, oc * axis.z * axis.x + axis.y * s, 0.0,
		oc * axis.x * axis.y + axis.z * s, oc * axis.y * axis.y + c,          oc * axis.y * axis.z - axis.x * s, 0.0,
		oc * axis.z * axis.x - axis.y * s, oc * axis.y * axis.z + axis.x * s, oc * axis.z * axis.z + c,			 0.0,
		0.0,                               0.0,                               0.0,								 1.0
	};
	return mat;
}

// multiply a matrix with a vector (assumes w component of vector is 1.0f) 
Vector matMultVector(vector<float> rotMat, Vector v)
{
	Vector outv;
	outv.x = rotMat[0]*v.x + rotMat[4]*v.y + rotMat[8]*v.z  + rotMat[12];
	outv.y = rotMat[1]*v.x + rotMat[5]*v.y + rotMat[9]*v.z  + rotMat[13];
	outv.z = rotMat[2]*v.x + rotMat[6]*v.y + rotMat[10]*v.z + rotMat[14];
	return outv;
}

void knockBack(CBaseEntity* target, Vector vel)
{
	float velCap = 0.0f;
	if (vel.Length() > velCap)
		vel = vel.Normalize()*velCap;
	if ((target->IsMonster() || target->IsPlayer()) && !target->IsMachine())
		target->pev->velocity = target->pev->velocity + vel;
}

enum impact_decal_type
{
	DECAL_NONE = -2,
	DECAL_BIGBLOOD = 0,
	DECAL_BIGSHOT,
	DECAL_BLOOD,
	DECAL_BLOODHAND,
	DECAL_BULLETPROOF,
	DECAL_GLASSBREAK,
	DECAL_LETTERS,
	DECAL_SMALLCRACK,
	DECAL_LARGECRACK,
	DECAL_LARGEDENT,
	DECAL_SMALLDENT,
	DECAL_DING,
	DECAL_RUST,
	DECAL_FEET,
	DECAL_GARGSTOMP,
	DECAL_GUASS,
	DECAL_GRAFITTI,
	DECAL_HANDICAP,
	DECAL_MOMMABLOB,
	DECAL_SMALLSCORTCH,
	DECAL_MEDIUMSCORTCH,
	DECAL_TINYSCORTCH,
	DECAL_OIL,
	DECAL_LARGESCORTCH,
	DECAL_SMALLSHOT,
	DECAL_NUMBERS,
	DECAL_TINYSCORTCH2,
	DECAL_SMALLSCORTCH2,
	DECAL_SPIT,
	DECAL_BIGABLOOD,
	DECAL_TARGET,
	DECAL_TIRE,
	DECAL_ABLOOD,
};

vector< vector<const char*> > g_decals = {
	{"{bigblood1", "{bigblood2"},
	{"{bigshot1", "{bigshot2", "{bigshot3", "{bigshot4", "{bigshot5"},
	{"{blood1", "{blood2", "{blood3", "{blood4", "{blood5", "{blood6", "{blood7", "{blood8"},
	{"{bloodhand1", "{bloodhand2", "{bloodhand3", "{bloodhand4", "{bloodhand5", "{bloodhand6"},
	{"{bproof1"},
	{"{break1", "{break2", "{break3"},
	{"{capsa", "{capsb", "{capsc", "{capsd", "{capse", "{capsf", "{capsg", "{capsh", "{capsi", "{capsj",
		"{capsk", "{capsl", "{capsm", "{capsn", "{capso", "{capsp", "{capsq", "{capsr", "{capss", "{capst",
		"{capsu", "{capsv", "{capsw", "{capsx", "{capsy", "{capsz"},
	{"{crack1", "{crack2"},
	{"{crack3", "{crack4"},
	{"{dent1", "{dent2"},
	{"{dent3", "{dent4", "{dent5", "{dent6"},
	{"{ding3", "{ding4", "{ding5", "{ding6", "{ding7", "{ding8", "{ding9"},
	{"{ding10", "{ding11"},
	{"{foot_l", "{foot_r"},
	{"{gargstomp"},
	{"{gaussshot1"},
	{"{graf001", "{graf002", "{graf003", "{graf004", "{graf005"},
	{"{handi"},
	{"{mommablob"},
	{"{ofscorch1", "{ofscorch2", "{ofscorch3"},
	{"{ofscorch4", "{ofscorch5", "{ofscorch6"},
	{"{ofsmscorch1", "{ofsmscorch2", "{ofsmscorch3"},
	{"{oil1", "{oil2"},
	{"{scorch1", "{scorch2", "{scorch3"},
	{"{shot1", "{shot2", "{shot3", "{shot4", "{shot5"},
	{"{small#s0", "{small#s1", "{small#s2", "{small#s3", "{small#s4",
		"{small#s5", "{small#s6", "{small#s7", "{small#s8", "{small#s9"},
	{"{smscorch1", "{smscorch2"},
	{"{smscorch3"},
	{"{spit1", "{spit2"},
	{"{spr_splt1", "{spr_splt2", "{spr_splt3"},
	{"{target", "{target2"},
	{"{tire1", "{tire2"},
	{"{yblood1", "{yblood2", "{yblood3", "{yblood4", "{yblood5", "{yblood6"},
};

int getDecal(int decalType)
{
	if (decalType < 0 || decalType >= DECAL_SPR_SPLT3)
		decalType = DECAL_GUNSHOT1;
		
	vector<const char*>& decals = g_decals[decalType];
	return DECAL_INDEX(decals[ RANDOM_LONG(0, decals.size()-1) ]);
}

void doomBulletImpact(Vector pos, Vector normal, CBaseEntity* phit)
{
	if (strcmp(STRING(phit->pev->classname), "item_barrel"))
		UTIL_Decal(phit->entindex(), pos, getDecal(DECAL_SMALLSHOT));

	// no idea why sprite is so far off on different angles...
	if (normal.z < -0.1f)
		normal = normal * 3.0f;
	else if (normal.z > 0.1f)
		normal = normal * 0;

	int sprIdx = MODEL_INDEX("sprites/doom/puff.spr");
	UTIL_ExplosionMsg(pos + normal * 4, sprIdx, 14, 10, 15);
}


void HitScan(CBaseEntity* attacker, Vector vecSrc, Vector dir, float spread, float damage)
{
	float range = 16384;
	
	Vector vecAiming = spreadDir(dir.Normalize(), spread, SPREAD_GAUSSIAN);

	// Do the bullet collision
	TraceResult tr;
	Vector vecEnd = vecSrc + vecAiming * range;
	//te_beampoints(vecSrc, vecEnd);
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, attacker->edict(), &tr );
	
	// do more fancy effects
	if( tr.flFraction < 1.0 )
	{
		if( tr.pHit  )
		{
			CBaseEntity* pHit = CBaseEntity::Instance( tr.pHit );
			
			if( pHit  ) 
			{							
				CBaseEntity* ent = CBaseEntity::Instance( tr.pHit );
					
				Vector attackDir = (tr.vecEndPos - vecSrc).Normalize();
				Vector angles = UTIL_VecToAngles(attackDir);
				MAKE_VECTORS(angles);
					
				// damage done before hitgroup multipliers
				
				ClearMultiDamage(); // fixes TraceAttack() crash for some reason
				pHit->TraceAttack(attacker->pev, damage, attackDir, &tr, DMG_BULLET);
				Vector oldVel = pHit->pev->velocity;
				ApplyMultiDamage(attacker->pev, attacker->pev);
				
				pHit->pev->velocity = oldVel; // prevent high damage from launching unless we ask for it (unless DMG_LAUNCH)

				knockBack(pHit, gpGlobals->v_forward*(100+damage));
				
				if (pHit->IsBSPModel() || !strcmp(STRING(pHit->pev->classname), "item_barrel")) 
				{
					//te_gunshotdecal(tr.vecEndPos, pHit, getDecal(DECAL_SMALLSHOT));
					doomBulletImpact(tr.vecEndPos, tr.vecPlaneNormal, pHit);
				}
			}
		}
	}
	
	// bullet tracer effects
	//te_tracer(vecSrc, tr.vecEndPos);
}


// Randomize the direction of a vector by some amount
// Max degrees = 360, which makes a full sphere
Vector spreadDir(Vector dir, float degrees, int spreadFunc)
{
	float spread = (degrees * M_PI / 180.0f) * 0.5f;
	float x, y;
	Vector vecAiming = dir;
	
	if (spreadFunc == SPREAD_GAUSSIAN) 
	{
		GetCircularGaussianSpread( x, y );
		x *= RANDOM_FLOAT(-spread, spread);
		y *= RANDOM_FLOAT(-spread, spread);
	} 
	else if (spreadFunc == SPREAD_UNIFORM) 
	{
		float c = RANDOM_FLOAT(0, M_PI*2); // random point on circle
		float r = RANDOM_FLOAT(-1, 1); // random radius
		x = cos(c) * r * spread;
		y = sin(c) * r * spread;
	}
	
	// get "up" vector relative to aim direction
	Vector pitAxis = CrossProduct(dir, Vector(0, 0, 1)).Normalize(); // get left vector of aim dir
	Vector yawAxis = CrossProduct(dir, pitAxis).Normalize(); // get up vector relative to aim dir
	
	// Apply rotation around arbitrary "up" axis
	vector<float> yawRotMat = rotationMatrix(yawAxis, x);
	vecAiming = matMultVector(yawRotMat, vecAiming).Normalize();
	
	// Apply rotation around "left/right" axis
	vector<float> pitRotMat = rotationMatrix(pitAxis, y);
	vecAiming = matMultVector(pitRotMat, vecAiming).Normalize();
			
	return vecAiming;
}

CBasePlayer* getAnyPlayer() 
{
	CBaseEntity* ent = NULL;
	do {
		ent = UTIL_FindEntityByClassname(ent, "player");
		if (ent ) {
			CBasePlayer* plr = (CBasePlayer*)(ent);
			return plr;
		}
	} while (ent );
	return NULL;
}