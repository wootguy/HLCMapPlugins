#include "extdll.h"
#include "util.h"
#include "doom_utils.h"

string getSteamID(CBasePlayer* plr)
{
	string steamId = GetPlayerUniqueId( plr->edict() );
	if (steamId == 'STEAM_ID_LAN') {
		steamId = plr->pev->netname;
	}
	return steamId;
}

// Will create a new state if the requested one does not exit
PlayerState* getPlayerState(CBasePlayer* plr)
{
	string steamId = getSteamID(plr);
	
	if ( !player_states.exists(steamId) )
	{
		PlayerState state;
		state.plr = plr;
		player_states[steamId] = state;
	}
	return cast<PlayerState*>( player_states[steamId] );
}


class Color
{ 
	uint8 r, g, b, a;
	Color() { r = g = b = a = 0; }
	Color(uint8 r, uint8 g, uint8 b) { this->r = r; this->g = g; this->b = b; this->a = 255; }
	Color(uint8 r, uint8 g, uint8 b, uint8 a) { this->r = r; this->g = g; this->b = b; this->a = a; }
	Color(float r, float g, float b, float a) { this->r = uint8(r); this->g = uint8(g); this->b = uint8(b); this->a = uint8(a); }
	Color (Vector v) { this->r = uint8(v.x); this->g = uint8(v.y); this->b = uint8(v.z); this->a = 255; }
	string ToString() { return "" + r + " " + g + " " + b + " " + a; }
	Vector getRGB() { return Vector(r, g, b); }
}

Color RED    = Color(255,0,0);
Color GREEN  = Color(0,255,0);
Color BLUE   = Color(0,0,255);
Color YELLOW = Color(255,255,0);
Color ORANGE = Color(255,127,0);
Color PURPLE = Color(127,0,255);
Color PINK   = Color(255,0,127);
Color TEAL   = Color(0,255,255);
Color WHITE  = Color(255,255,255);
Color BLACK  = Color(0,0,0);
Color GRAY  = Color(127,127,127);

// convert output from Vector.ToString() back into a Vector
Vector parseVector(string s) {
	vector<string> values = s.Split(" ");
	Vector v(0,0,0);
	if (values.length() > 0) v.x = atof( values[0] );
	if (values.length() > 1) v.y = atof( values[1] );
	if (values.length() > 2) v.z = atof( values[2] );
	return v;
}

void te_projectile(Vector pos, Vector velocity, CBaseEntity* owner=NULL, 
	string model="models/grenade.mdl", uint8 life=1, 
	NetworkMessageDest msgType=MSG_BROADCAST, edict_t* dest=NULL)
{
	int ownerId = owner  == NULL  ? 0 : owner->entindex();
	MESSAGE_BEGIN m(msgType, SVC_TEMPENTITY, dest);
	m.WRITE_BYTE(TE_PROJECTILE);
	m.WRITE_COORD(pos.x);
	m.WRITE_COORD(pos.y);
	m.WRITE_COORD(pos.z);
	m.WRITE_COORD(velocity.x);
	m.WRITE_COORD(velocity.y);
	m.WRITE_COORD(velocity.z);
	m.WRITE_SHORT(g_engfuncs.pfnModelIndex(model));
	m.WRITE_BYTE(life);
	m.WRITE_BYTE(ownerId);
	m.MESSAGE_END();
}
void te_explosion(Vector pos, string sprite="sprites/zerogxplode.spr", 
	int scale=10, int frameRate=15, int flags=0,
	NetworkMessageDest msgType=MSG_BROADCAST, edict_t* dest=NULL)
{
	MESSAGE_BEGIN m(msgType, SVC_TEMPENTITY, dest);
	m.WRITE_BYTE(TE_EXPLOSION);
	m.WRITE_COORD(pos.x);
	m.WRITE_COORD(pos.y);
	m.WRITE_COORD(pos.z);
	m.WRITE_SHORT(g_engfuncs.pfnModelIndex(sprite));
	m.WRITE_BYTE(scale);
	m.WRITE_BYTE(frameRate);
	m.WRITE_BYTE(flags);
	m.MESSAGE_END();
}
void te_killbeam(CBaseEntity* target, 
	NetworkMessageDest msgType=MSG_BROADCAST, edict_t* dest=NULL)
{
	MESSAGE_BEGIN m(msgType, SVC_TEMPENTITY, dest);
	m.WRITE_BYTE(TE_KILLBEAM);
	m.WRITE_SHORT(target->entindex());
	m.MESSAGE_END();
}
void te_beamentpoint(CBaseEntity* target, Vector end, 
	string sprite="sprites/laserbeam.spr", int frameStart=0, 
	int frameRate=100, int life=10, int width=32, int noise=1, 
	Color c=PURPLE, int scroll=32,
	NetworkMessageDest msgType=MSG_BROADCAST, edict_t* dest=NULL)
{
	MESSAGE_BEGIN m(msgType, SVC_TEMPENTITY, dest);
	m.WRITE_BYTE(TE_BEAMENTPOINT);
	m.WRITE_SHORT(target->entindex());
	m.WRITE_COORD(end.x);
	m.WRITE_COORD(end.y);
	m.WRITE_COORD(end.z);
	m.WRITE_SHORT(g_engfuncs.pfnModelIndex(sprite));
	m.WRITE_BYTE(frameStart);
	m.WRITE_BYTE(frameRate);
	m.WRITE_BYTE(life);
	m.WRITE_BYTE(width);
	m.WRITE_BYTE(noise);
	m.WRITE_BYTE(c.r);
	m.WRITE_BYTE(c.g);
	m.WRITE_BYTE(c.b);
	m.WRITE_BYTE(c.a); // actually brightness
	m.WRITE_BYTE(scroll);
	m.MESSAGE_END();
}
void te_beampoints(Vector start, Vector end, string sprite="sprites/laserbeam.spr", uint8 frameStart=0, uint8 frameRate=100, uint8 life=20, uint8 width=2, uint8 noise=0, Color c=GREEN, uint8 scroll=32, NetworkMessageDest msgType=MSG_BROADCAST, edict_t* dest=NULL) { MESSAGE_BEGIN m(msgType, SVC_TEMPENTITY, dest);m.WRITE_BYTE(TE_BEAMPOINTS);m.WRITE_COORD(start.x);m.WRITE_COORD(start.y);m.WRITE_COORD(start.z);m.WRITE_COORD(end.x);m.WRITE_COORD(end.y);m.WRITE_COORD(end.z);m.WRITE_SHORT(g_engfuncs.pfnModelIndex(sprite));m.WRITE_BYTE(frameStart);m.WRITE_BYTE(frameRate);m.WRITE_BYTE(life);m.WRITE_BYTE(width);m.WRITE_BYTE(noise);m.WRITE_BYTE(c.r);m.WRITE_BYTE(c.g);m.WRITE_BYTE(c.b);m.WRITE_BYTE(c.a);m.WRITE_BYTE(scroll);m.MESSAGE_END(); }
void _te_decal(Vector pos, CBaseEntity* plr, CBaseEntity* brushEnt, string decal, NetworkMessageDest msgType, edict_t* dest, int decalType) { int decalIdx = g_engfuncs.pfnDecalIndex(decal); int entIdx = brushEnt  == NULL  ? 0 : brushEnt->entindex(); if (decalIdx == -1) {  if (plr ) decalIdx = 0;  else  { println("Invalid decal: " + decalIdx); return;  } } if (decalIdx > 511) {  println("Decal index too high (" + decalIdx + ")! Max decal index is 511.");  return; } if (decalIdx > 255) {  decalIdx -= 255;  if (decalType == TE_DECAL) decalType = TE_DECALHIGH;  else if (decalType == TE_WORLDDECAL) decalType = TE_WORLDDECALHIGH;  else println("Decal type " + decalType + " doesn't support indicies > 255"); } if (decalType == TE_DECAL && entIdx == 0) decalType = TE_WORLDDECAL; if (decalType == TE_DECALHIGH && entIdx == 0) decalType = TE_WORLDDECALHIGH; MESSAGE_BEGIN m(msgType, SVC_TEMPENTITY, dest); m.WRITE_BYTE(decalType); if (plr ) m.WRITE_BYTE(plr->entindex()); m.WRITE_COORD(pos.x); m.WRITE_COORD(pos.y); m.WRITE_COORD(pos.z); switch(decalType) {  case TE_DECAL: case TE_DECALHIGH: m.WRITE_BYTE(decalIdx); m.WRITE_SHORT(entIdx); break;  case TE_GUNSHOTDECAL: case TE_PLAYERDECAL: m.WRITE_SHORT(entIdx); m.WRITE_BYTE(decalIdx); break;  default: m.WRITE_BYTE(decalIdx); } m.MESSAGE_END(); }
void te_decal(Vector pos, CBaseEntity* brushEnt=NULL, string decal="{handi", NetworkMessageDest msgType=MSG_BROADCAST, edict_t* dest=NULL) { _te_decal(pos, NULL, brushEnt, decal, msgType, dest, TE_DECAL); }
void te_gunshotdecal(Vector pos, CBaseEntity* brushEnt=NULL, string decal="{handi", NetworkMessageDest msgType=MSG_BROADCAST, edict_t* dest=NULL) { _te_decal(pos, NULL, brushEnt, decal, msgType, dest, TE_GUNSHOTDECAL); }
void te_tracer(Vector start, Vector end, NetworkMessageDest msgType=MSG_BROADCAST, edict_t* dest=NULL) { MESSAGE_BEGIN m(msgType, SVC_TEMPENTITY, dest);m.WRITE_BYTE(TE_TRACER);m.WRITE_COORD(start.x);m.WRITE_COORD(start.y);m.WRITE_COORD(start.z);m.WRITE_COORD(end.x);m.WRITE_COORD(end.y);m.WRITE_COORD(end.z);m.MESSAGE_END(); }
void te_dlight(Vector pos, uint8 radius=16, Color c=PURPLE, uint8 life=255, uint8 decayRate=4, NetworkMessageDest msgType=MSG_BROADCAST, edict_t* dest=NULL) { MESSAGE_BEGIN m(msgType, SVC_TEMPENTITY, dest);m.WRITE_BYTE(TE_DLIGHT);m.WRITE_COORD(pos.x);m.WRITE_COORD(pos.y);m.WRITE_COORD(pos.z);m.WRITE_BYTE(radius);m.WRITE_BYTE(c.r);m.WRITE_BYTE(c.g);m.WRITE_BYTE(c.b);m.WRITE_BYTE(life);m.WRITE_BYTE(decayRate);m.MESSAGE_END(); }
void te_sprite(Vector pos, string sprite="sprites/zerogxplode.spr", 
	uint8 scale=10, uint8 alpha=200, 
	NetworkMessageDest msgType=MSG_BROADCAST, edict_t* dest=NULL)
{
	MESSAGE_BEGIN m(msgType, SVC_TEMPENTITY, dest);
	m.WRITE_BYTE(TE_SPRITE);
	m.WRITE_COORD(pos.x);
	m.WRITE_COORD(pos.y);
	m.WRITE_COORD(pos.z);
	m.WRITE_SHORT(g_engfuncs.pfnModelIndex(sprite));
	m.WRITE_BYTE(scale);
	m.WRITE_BYTE(alpha);
	m.MESSAGE_END();
}
void te_model(Vector pos, Vector velocity, float yaw=0, 
	string model="models/agibs.mdl", uint8 bounceSound=2, uint8 life=32,
	NetworkMessageDest msgType=MSG_BROADCAST, edict_t* dest=NULL)
{

	MESSAGE_BEGIN m(msgType, SVC_TEMPENTITY, dest);
	m.WRITE_BYTE(TE_MODEL);
	m.WRITE_COORD(pos.x);
	m.WRITE_COORD(pos.y);
	m.WRITE_COORD(pos.z);
	m.WRITE_COORD(velocity.x);
	m.WRITE_COORD(velocity.y);
	m.WRITE_COORD(velocity.z);
	m.WriteAngle(yaw);
	m.WRITE_SHORT(g_engfuncs.pfnModelIndex(model));
	m.WRITE_BYTE(bounceSound);
	m.WRITE_BYTE(life);
	m.MESSAGE_END();
}
void te_spray(Vector pos, Vector dir, string sprite="sprites/bubble.spr", 
	uint8 count=8, uint8 speed=127, uint8 noise=255, uint8 rendermode=0,
	NetworkMessageDest msgType=MSG_BROADCAST, edict_t* dest=NULL)
{
	MESSAGE_BEGIN m(msgType, SVC_TEMPENTITY, dest);
	m.WRITE_BYTE(TE_SPRAY);
	m.WRITE_COORD(pos.x);
	m.WRITE_COORD(pos.y);
	m.WRITE_COORD(pos.z);
	m.WRITE_COORD(dir.x);
	m.WRITE_COORD(dir.y);
	m.WRITE_COORD(dir.z);
	m.WRITE_SHORT(g_engfuncs.pfnModelIndex(sprite));
	m.WRITE_BYTE(count);
	m.WRITE_BYTE(speed);
	m.WRITE_BYTE(noise);
	m.WRITE_BYTE(rendermode);
	m.MESSAGE_END();
}

void te_multigunshot(Vector pos, Vector dir, float spreadX=512.0f, 
	float spreadY=512.0f, uint8 count=8, string decal="{shot4",
	NetworkMessageDest msgType=MSG_BROADCAST, edict_t* dest=NULL)
{   
	MESSAGE_BEGIN m(msgType, SVC_TEMPENTITY, dest);
	m.WRITE_BYTE(TE_MULTIGUNSHOT);
	m.WRITE_COORD(pos.x);
	m.WRITE_COORD(pos.y);
	m.WRITE_COORD(pos.z);
	m.WRITE_COORD(dir.x);
	m.WRITE_COORD(dir.y);
	m.WRITE_COORD(dir.z);
	m.WRITE_COORD(spreadX);
	m.WRITE_COORD(spreadY);
	m.WRITE_BYTE(count);
	m.WRITE_BYTE(g_engfuncs.pfnDecalIndex(decal));
	m.MESSAGE_END();
}
void te_bloodsprite(Vector pos, string sprite1="sprites/bloodspray.spr",
	string sprite2="sprites/blood.spr", uint8 color=70, uint8 scale=3,
	NetworkMessageDest msgType=MSG_BROADCAST, edict_t* dest=NULL)
{
	MESSAGE_BEGIN m(msgType, SVC_TEMPENTITY, dest);
	m.WRITE_BYTE(TE_BLOODSPRITE);
	m.WRITE_COORD(pos.x);
	m.WRITE_COORD(pos.y);
	m.WRITE_COORD(pos.z);
	m.WRITE_SHORT(g_engfuncs.pfnModelIndex(sprite1));
	m.WRITE_SHORT(g_engfuncs.pfnModelIndex(sprite2));
	m.WRITE_BYTE(color);
	m.WRITE_BYTE(scale);
	m.MESSAGE_END();
}

TraceResult TraceLook(CBasePlayer* plr, float dist=128, bool bigHull=false)
{
	Vector vecSrc = plr.GetGunPosition();
	MAKE_VECTORS( plr->pev->v_angle ); // todo: monster angles
	
	TraceResult tr;
	Vector vecEnd = vecSrc + gpGlobals->v_forward * dist;
	if (bigHull)
		UTIL_TraceHull( vecSrc, vecEnd, dont_ignore_monsters, head_hull, plr->edict(), tr );
	else
		UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, plr->edict(), tr );
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
	DECAL_TYPES,
}

vector< vector<string> > g_decals = {
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

void knockBack(CBaseEntity* target, Vector vel)
{
	float velCap = 0.0f;
	if (vel.Length() > velCap)
		vel = vel.Normalize()*velCap;
	if ((target.IsMonster() || target.IsPlayer()) && !target.IsMachine())
		target->pev->velocity = target->pev->velocity + vel;
}

string getDecal(int decalType)
{
	if (decalType < 0 || decalType >= int(g_decals.length()))
		decalType = DECAL_SMALLSHOT;
		
	vector<string> decals = g_decals[decalType];
	return decals[ RANDOM_LONG(0, decals.length()-1) ];
}

enum spread_func
{
	SPREAD_GAUSSIAN,
	SPREAD_UNIFORM,
}

void HitScan(CBaseEntity* attacker, Vector vecSrc, Vector dir, float spread, float damage)
{
	float range = 16384;
	
	Vector vecAiming = spreadDir(dir.Normalize(), spread, SPREAD_GAUSSIAN);

	// Do the bullet collision
	TraceResult tr;
	Vector vecEnd = vecSrc + vecAiming * range;
	//te_beampoints(vecSrc, vecEnd);
	UTIL_TraceLine( vecSrc, vecEnd, dont_ignore_monsters, attacker->edict(), tr );
	
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
				TraceAttack(pHit, attacker.pev, damage, attackDir, tr, DMG_BULLET);
				
				Vector oldVel = pHit->pev->velocity;
				
				// set both classes in case this a pvp map where classes are always changing
				int oldClass1 = attacker.GetClassification(0);
				int oldClass2 = pHit.GetClassification(0);
				
				// SetClassification crashes in SC 5.22
				attacker.KeyValue("classify", CLASS_PLAYER);
				pHit.KeyValue("classify", CLASS_ALIEN_MILITARY);
				
				g_WeaponFuncs.ApplyMultiDamage(attacker.pev, attacker.pev);
				
				attacker.KeyValue("classify", oldClass1);
				pHit.KeyValue("classify", oldClass2);
				
				pHit->pev->velocity = oldVel; // prevent high damage from launching unless we ask for it (unless DMG_LAUNCH)

				knockBack(pHit, gpGlobals->v_forward*(100+damage)*g_world_scale);
				
				if (pHit->IsBSPModel() || pHit->pev->classname == "item_barrel") 
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

Vector g_vecAttackDir;

float DamageForce( CBaseEntity* ent, float damage )
{ 
	float force = damage * ((32 * 32 * 72.0) / (ent->pev->size.x * ent->pev->size.y * ent->pev->size.z)) * 5;
	return force > 1000.0f ? 1000.0f : force;
}

// armor should only absord a reasonable of damage, not all of it.
int doomTakeDamage(CBaseEntity* ent, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, int bitsDamageType)
{
	if (!ent.IsPlayer() || !ent->IsAlive())
		return ent.TakeDamage( pevAttacker, pevAttacker, flDamage, bitsDamageType);
		
	flDamage *= g_dmgScale.GetFloat();
	
	// disable friendly fire
	CBaseEntity* attacker = CBaseEntity::Instance( pevAttacker );
	if (ent.IsPlayer() && attacker.IsPlayer() && ent->entindex() != attacker->entindex() && !g_friendly_fire)
		return 0;
	
	float	flTake;
	Vector	vecDir;

	if (ent->pev->takedamage == DAMAGE_NO || ent->pev->flags & FL_GODMODE != 0)
		return 0;

	if ( ent->pev->deadflag == DEAD_NO )
	{
		// no pain sound during death animation.
		EMIT_SOUND_DYN(ent->edict(), CHAN_STATIC, fixPath("doom/dsplpain.wav"), 1.0f, 1.0f, 0, 100);
		g_PlayerFuncs.ScreenFade(ent, Vector(255, 0, 0), 0.2f, 0, 32, FFADE_IN);
	}

	//!!!LATER - make armor consideration here!
	flTake = flDamage - 1; // account for multidamage

	// grab the vector of the incoming attack. ( pretend that the inflictor is a little lower than it really is, so the body will tend to fly upward a bit).
	vecDir = Vector( 0, 0, 0 );
	if (pevInflictor )
	{
		CBaseEntity* pInflictor = CBaseEntity::Instance( pevInflictor );
		if (pInflictor )
		{
			vecDir = ( pInflictor.Center() - Vector ( 0, 0, 10 ) - ent.Center() ).Normalize();
			vecDir = g_vecAttackDir = vecDir.Normalize();
		}
	}

	// if this is a player, move him around!
	if ( pevInflictor  && (ent->pev->movetype == MOVETYPE_WALK) && (pevAttacker  == NULL  || pevAttacker.solid != SOLID_TRIGGER) )
		ent->pev->velocity = ent->pev->velocity + vecDir * -DamageForce(ent, flDamage );

	// do the damage	
	float armorAbsorb = Math.min(ent->pev->armorvalue, int(flTake*0.33f));
	flTake -= armorAbsorb;
	ent->pev->armorvalue -= armorAbsorb;
	ent->pev->health -= flTake;
	
	// do minimum damage needed to show the red directional hud thingies
	ClearMultiDamage();
	g_WeaponFuncs.AddMultiDamage(pevInflictor, ent, 1.0f, bitsDamageType);
	g_WeaponFuncs.ApplyMultiDamage(pevInflictor, pevAttacker);

	if ( ent->pev->health <= 0 )
	{
		if ( bitsDamageType & DMG_ALWAYSGIB != 0 )
		{
			ent.Killed( pevAttacker, GIB_ALWAYS );
		}
		else if ( bitsDamageType & DMG_NEVERGIB != 0 )
		{
			ent.Killed( pevAttacker, GIB_NEVER );
		}
		else
		{
			// TODO: What's the normal gib damage amount?
			ent.Killed( pevAttacker, GIB_NORMAL );
		}
		return 0;
	}

	return 1;
}

void TraceAttack(CBaseEntity* victim, entvars_t* pevAttacker, float flDamage, Vector vecDir, TraceResult tr, int bitsDamageType)
{
	Vector vecOrigin = tr.vecEndPos - vecDir * 4;
	
	g_vecAttackDir = vecDir;

	if ( victim->pev->takedamage != DAMAGE_NO )
	{
		if (victim.IsPlayer())
		{
			doomTakeDamage(victim, pevAttacker, pevAttacker, flDamage, bitsDamageType);
		}
		else
			g_WeaponFuncs.AddMultiDamage( pevAttacker, victim, flDamage, bitsDamageType );

		int blood = victim.BloodColor();
		
		if ( blood != DONT_BLEED )
		{
			te_bloodsprite(vecOrigin, fixPath("sprites/doom/blud.spr"), "sprites/blood.spr", 70, 5);
			//te_explosion(vecOrigin, "sprites/doom/blud.spr", 10, 10, 15);
		}
	}
}

// a basic set of directions for a sphere (up/down/left/right/front/back with 1 in-between step)
// This isn't good enough for large explosions, but hopefully FindEntityInSphere will work at that point.
vector<Vector> sphereDirs = {Vector(1,0,0).Normalize(), Vector(0,1,0).Normalize(), Vector(0,0,1).Normalize(),
							  Vector(-1,0,0).Normalize(), Vector(0,-1,0).Normalize(), Vector(0,0,-1).Normalize(),
							  Vector(1,1,0).Normalize(), Vector(-1,1,0).Normalize(), Vector(1,-1,0).Normalize(), Vector(-1,-1,0).Normalize(),
							  Vector(1,0,1).Normalize(), Vector(-1,0,1).Normalize(), Vector(1,0,-1).Normalize(), Vector(-1,0,-1).Normalize(),
							  Vector(0,1,1).Normalize(), Vector(0,-1,1).Normalize(), Vector(0,1,-1).Normalize(), Vector(0,-1,-1).Normalize()};

void RadiusDamage( Vector vecSrc, entvars_t* pevInflictor, entvars_t* pevAttacker, float flDamage, float flRadius, int iClassIgnore, int bitsDamageType )
{
	CBaseEntity* pEntity = NULL;
	TraceResult	tr;
	float flAdjustedDamage, falloff;
	Vector vecSpot;

	if ( flRadius > 0 )
		falloff = flDamage / flRadius;
	else
		falloff = 1.0;

	bool bInWater = (g_engfuncs.pfnPointContents(vecSrc) == CONTENTS_WATER);

	vecSrc.z += 1;// in case grenade is lying on the ground

	if ( pevAttacker  == NULL  )
		*pevAttacker = *pevInflictor;
		
	dictionary attacked;
	// iterate on all entities in the vicinity.
	while ((*pEntity = UTIL_FindEntityInSphere( pEntity, vecSrc, flRadius, "*", "classname" )) != NULL)
	{
		attacked[pEntity->entindex()] = true;
		if ( pEntity->pev->takedamage != DAMAGE_NO )
		{
			// UNDONE: this should check a damage mask, not an ignore
			if ( iClassIgnore != CLASS_NONE && pEntity.Classify() == iClassIgnore )
			{// houndeyes don't hurt other houndeyes with their attack
				continue;
			}

			// blast's don't tavel into || out of water
			if (bInWater && pEntity->pev->waterlevel == 0)
				continue;
			if (!bInWater && pEntity->pev->waterlevel == 3)
				continue;

			vecSpot = pEntity.BodyTarget( vecSrc );
			
			UTIL_TraceLine( vecSrc, vecSpot, dont_ignore_monsters, CBaseEntity::Instance(pevInflictor)->edict(), tr );

			if ( tr.flFraction == 1.0 || CBaseEntity::Instance(tr.pHit)->entindex() == CBaseEntity::Instance(pEntity->edict())->entindex() )
			{// the explosion can 'see' this entity, so hurt them!
				if (tr.fStartSolid != 0)
				{
					// if we're stuck inside them, fixup the position && distance
					tr.vecEndPos = vecSrc;
					tr.flFraction = 0.0;
				}
				
				// decrease damage for an ent that's farther from the bomb.
				flAdjustedDamage = ( vecSrc - tr.vecEndPos ).Length() * falloff;
				flAdjustedDamage = flDamage - flAdjustedDamage;
			
				if ( flAdjustedDamage < 0 )
				{
					flAdjustedDamage = 0;
				}
			
				if (tr.flFraction != 1.0)
				{
					g_WeaponFuncs.ClearMultiDamage( );
					TraceAttack(pEntity, pevAttacker, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize( ), tr, bitsDamageType );
					g_WeaponFuncs.ApplyMultiDamage( pevInflictor, pevAttacker );
				}
				else
				{
					doomTakeDamage(pEntity, pevAttacker, pevAttacker, flAdjustedDamage, bitsDamageType);
				}
			}
		}
	}

	// Now cast a few rays to make sure we hit the obvious targets. This is needed 
	// for things like tall func_breakables. For example, if the origin is at the 
	// bottom but the explosion origin is at the top. FindEntityInSphere won't  
	// detect it even if the explosion is touching the surface of the brush.
	for (uint i = 0; i < sphereDirs.size(); i++)
	{
		//te_beampoints(vecSrc, vecSrc + sphereDirs[i]*flRadius);
		UTIL_TraceLine( vecSrc, vecSrc + sphereDirs[i]*flRadius, dont_ignore_monsters, CBaseEntity::Instance(pevInflictor)->edict(), tr );
		CBaseEntity* pHit = CBaseEntity::Instance(tr.pHit);
		if (pHit  == NULL  || attacked.exists(pHit->entindex()) || pHit->entindex() == 0)
			continue;
			
		attacked[pHit->entindex()] = true;
		
		if (tr.fStartSolid != 0)
		{
			// if we're stuck inside them, fixup the position && distance
			tr.vecEndPos = vecSrc;
			tr.flFraction = 0.0;
		}
		
		// decrease damage for an ent that's farther from the bomb.
		flAdjustedDamage = ( vecSrc - tr.vecEndPos ).Length() * falloff;
		flAdjustedDamage = flDamage - flAdjustedDamage;
	
		if ( flAdjustedDamage < 0 )
		{
			flAdjustedDamage = 0;
		}
	
		if (tr.flFraction != 1.0)
		{
			g_WeaponFuncs.ClearMultiDamage( );
			TraceAttack(pHit, pevAttacker, flAdjustedDamage, (tr.vecEndPos - vecSrc).Normalize( ), tr, bitsDamageType );
			g_WeaponFuncs.ApplyMultiDamage( pevInflictor, pevAttacker );
		}
		else
		{
			doomTakeDamage(pHit, pevAttacker, pevAttacker, flAdjustedDamage, bitsDamageType);
		}
	}
	
}

void doomBulletImpact(Vector pos, Vector normal, CBaseEntity* phit)
{
	if (phit->pev->classname != "item_barrel")
		te_decal(pos, phit, getDecal(DECAL_SMALLSHOT));
	// no idea why sprite is so far off on different angles...
	if (normal.z < -0.1f)
		normal = normal * 3.0f;
	else if (normal.z > 0.1f)
		normal = normal * 0;
	te_explosion(pos + normal*4, fixPath("sprites/doom/puff.spr"), 14, 10, 15);
}

// Randomize the direction of a vector by some amount
// Max degrees = 360, which makes a full sphere
Vector spreadDir(Vector dir, float degrees, int spreadFunc=SPREAD_UNIFORM)
{
	float spread = Math.DegreesToRadians(degrees) * 0.5f;
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
		float c = RANDOM_FLOAT(0, Math.PI*2); // random point on circle
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
		*ent = UTIL_FindEntityByClassname(ent, "player");
		if (ent ) {
			CBasePlayer* plr = (CBasePlayer*)(ent);
			return plr;
		}
	} while (ent );
	return NULL;
}