
class CEnvWarpball : ScriptBaseEntity
{
	CBeam *pBeam;
	string m_iszMaster;
	
	void Precache()
	{
		BaseClass.Precache();
		PRECACHE_MODEL("sprites/lgtning.spr" );
		PRECACHE_MODEL("sprites/Fexplo1.spr" );
		PRECACHE_MODEL("sprites/XFlare1.spr" );
			
		PRECACHE_SOUND( "debris/beamstart2.wav" );
		PRECACHE_SOUND( "debris/beamstart7.wav" );
	}
	
	bool KeyValue( const string&szKey, const string&szValue )
	{
		if(szKey == "master")
		{
			m_iszMaster = szValue;
			return true;
		}
		else 
		{
			return BaseClass.KeyValue( szKey, szValue );
		}
	}
	
	void Spawn()
	{
		Precache();
	}
	
	int ObjectCaps()
	{
		return BaseClass.ObjectCaps() & ~FCAP_ACROSS_TRANSITION;
	}
	
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
	{
	
	if( !m_iszMaster.IsEmpty() && !g_EntityFuncs.IsMasterTriggered( m_iszMaster, self ) )
	{	
		return;
	}
	
	int iTimes = 0;
	int iDrawn = 0;
	TraceResult tr;
	Vector vecDest;
	while (iDrawn < int(pev->frags) && iTimes < int(pev->frags * 3)) // try to draw <frags> beams, but give up after 3x<frags> tries.
	{
		vecDest = pev->health * (Vector(RANDOM_FLOAT(-1,1), RANDOM_FLOAT(-1,1), RANDOM_FLOAT(-1,1)).Normalize());
		TRACE_LINE( pev->origin, pev->origin + vecDest, ignore_monsters, NULL, tr);
		
		if (tr.flFraction != 1.0)
		{
			// we hit something.
			iDrawn++;
			*pBeam = CBaseEntity::CreateBeam("sprites/lgtning.spr",200);
			pBeam.PointsInit( pev->origin, tr.vecEndPos );
			pBeam.SetColor( 197, 243, 169 );
			pBeam.SetNoise( 65 );
			pBeam.SetBrightness( 150 );
			pBeam.SetWidth( 18 );
			g_Scheduler.SetTimeout( "BeamRemove", 1.0f, *pBeam); 
			pBeam.SetScrollRate( 35 );
		}
		iTimes++;
	}
	EMIT_SOUND( edict(), CHAN_BODY, "debris/beamstart2.wav", 1.0f, ATTN_NORM );
	//EMIT_SOUND( edict(), CHAN_STATIC, "../media/valve.mp3", 1.0f, ATTN_NONE );
	
	CSprite *pSpr = CBaseEntity::CreateSprite( "sprites/Fexplo1.spr", pev->origin, true );
	pSpr.AnimateAndDie( 10 );
	pSpr.SetTransparency(kRenderGlow,  77, 210, 130,  255, kRenderFxNoDissipation);

	CSprite *pSpr2 = CBaseEntity::CreateSprite( "sprites/XFlare1.spr", pev->origin, true );
	pSpr2.AnimateAndDie( 10 );
	pSpr2.SetTransparency(kRenderGlow,  184, 250, 214,  255, kRenderFxNoDissipation);
	
	g_Scheduler.SetTimeout( "WarpballThink", 0.5f, *self); 
	}
}

void BeamRemove(CBeam *pBeam)
	{
		UTIL_Remove( pBeam );
	}
	
void WarpballThink(CBaseEntity *pEntity)
	{
		EMIT_SOUND( pEntity->edict(), CHAN_ITEM, "debris/beamstart7.wav", 1, ATTN_NORM );
		pEntity.SUB_UseTargets( *pEntity, USE_TOGGLE, 0);
	}

void RegisterEnvWarpball()
{
	g_CustomEntityFuncs.RegisterCustomEntity("CEnvWarpball", "env_warpball");
}