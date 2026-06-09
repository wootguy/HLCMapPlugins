#include "extdll.h"
#include "util.h"
#include "CBaseAnimating.h"

#define USE_KILL 4

// . Custom entity for echoes map series. 

/*
env_model_coop
Author: kmkz (e-mail: al_basualdo*hotmail.com )
this plays an animation. Similar to env_model of SOHL engine.

--------------------------------------
model: "Model name"
Model that will be displayed. 
m_iszSequence_On: "Sequence when on"
Name of the sequence that will play when it is on.
m_iszSequence_Off: "Sequence when off"
Name of the sequence that will play when it is off.
m_iAction_On: "Behaviour when on"
Behaviour of the enity when on, the are 3 options:
	0: "Freeze when sequence ends"
	The sequence will play once && then will stay quiet.
	1: "Loop"
	The sequence will play until the end && then will repeat.
	2: "Change state when sequence ends"
	The sequence will play once && then will the the oposite sequence 
m_iAction_Off: "Behaviour when off"
	same as above for the off sequence.
---------------------------------------
flags:
1: "Initially off"
instead of playing the on sequence this will play the off sequence
2: "Drop to floor"
Instead of floating in air this will drop to floor. 
4: "Solid"
Instead of being not solid this entity will be solid. 
---------------------------------------
note: if the sequence name is !none then no sequence will be played && the entity will stay frozen.
*/

enum AnimatingGenericSequenceSpawnFlag
{
	SF_AG_INITIALLY_OFF = 1 << 0,
	SF_AG_DROP_TO_FLOOR = 2 << 0,
	SF_AG_SOLID = 4 << 0,
};

class CEnvModelCoop : public CBaseAnimating
{
public:
	string_t m_iszSequence_Off;
	string_t m_iszSequence_On;
	int m_iAction_Off;
	int m_iAction_On;
	string_t sequence_active; //sequence that should be played
	int action_mode;
	Vector Vminhullsize;
	Vector Vmaxhullsize;
	string_t m_iszMaster;
	
	void Precache()
	{
		CBaseAnimating::Precache();
		PRECACHE_MODEL(STRING(pev->model));
	}
	
	void KeyValue(KeyValueData* pkvd) override
	{
		if (FStrEq(pkvd->szKeyName, "m_iszSequence_Off"))
		{
			m_iszSequence_Off = ALLOC_STRING(pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
		else if(FStrEq(pkvd->szKeyName, "m_iszSequence_On"))
		{
			m_iszSequence_On = ALLOC_STRING(pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
		else if(FStrEq(pkvd->szKeyName, "m_iAction_Off"))
		{
			m_iAction_Off = atoi (pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
		else if(FStrEq(pkvd->szKeyName, "m_iAction_On"))
		{
			m_iAction_On = atoi (pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
		else if(FStrEq(pkvd->szKeyName, "minhullsize"))
		{
			Vminhullsize = UTIL_ParseVector(pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
		else if(FStrEq(pkvd->szKeyName, "maxhullsize"))
		{
			Vmaxhullsize = UTIL_ParseVector(pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
		else if(FStrEq(pkvd->szKeyName, "master"))
		{
			m_iszMaster = ALLOC_STRING(pkvd->szValue);
			pkvd->fHandled = TRUE;
		}
		else 
		{
			CBaseAnimating::KeyValue(pkvd);
		}
	}
	
	void Spawn()
	{
		Precache();
		SET_MODEL(edict(), STRING(pev->model));
		InitBoneControllers(); 					// used to avoid having rotated body parts.
		SetUse(&CEnvModelCoop::TriggerUse);
		
		if (FBitSet(pev->spawnflags, SF_AG_INITIALLY_OFF ))	{sequence_active = m_iszSequence_Off;}	else {sequence_active = m_iszSequence_On;}
		if (FBitSet(pev->spawnflags, SF_AG_DROP_TO_FLOOR ))	{pev->movetype = MOVETYPE_TOSS;}	else {pev->movetype = MOVETYPE_NONE;}
		if (FBitSet(pev->spawnflags, SF_AG_SOLID  ))		{pev->solid = SOLID_SLIDEBOX; UTIL_SetSize(pev, Vminhullsize, Vmaxhullsize);} 		else {pev->solid = SOLID_NOT;}
		
		if (!strcmp(STRING(sequence_active), "!none"))
		{
			pev->sequence = -1;
		}
		else
		{
			Use(NULL, NULL, USE_SET ,0.0); 			// initialize the entity
		}
	}
	
	void TriggerUse(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue)
	{
		pev->frame = 0;
		
		if( !m_iszMaster && !UTIL_IsMasterTriggered( m_iszMaster, this ) && useType != USE_SET)
		{	
			return;
		}
		
		if (useType == USE_OFF)
		{
			sequence_active = m_iszSequence_Off;
			action_mode = m_iAction_Off;
		}
		
		if (useType == USE_ON)
		{
			sequence_active = m_iszSequence_On;
			action_mode = m_iAction_On;
		}
		
		if (useType == USE_TOGGLE) // switch between off && on status then activate
		{
			if (sequence_active == m_iszSequence_Off)
			{
				sequence_active = m_iszSequence_On;
				action_mode = m_iAction_On;
			}
			else
			{
				sequence_active = m_iszSequence_Off;
				action_mode = m_iAction_Off;
			}
		}
		
		if (useType == USE_KILL)
		{
			UTIL_Remove( this );
		}
		
		if (useType == USE_SET) // initializes the entity
		{
			if (sequence_active == m_iszSequence_Off)
			{
				sequence_active = m_iszSequence_Off;
				action_mode = m_iAction_Off;
			}
			else
			{
				sequence_active = m_iszSequence_On;
				action_mode = m_iAction_On;
			}
		}
		
		ActivateSequence (sequence_active, action_mode);
	}
	
	// play the sequence
	void ActivateSequence (string_t sequence_name, int acti_mode)
	{
		pev->frame = 0;
		if ( LookupSequence(STRING(sequence_name)) != -1 ) // check if the animation name is valid
		{
			pev->sequence = LookupSequence(STRING(sequence_name));
		}
		else
		{
			pev->sequence = 0;
		}
		
		ResetSequenceInfo();
		if ( acti_mode != 0) 
		{
			m_fSequenceLoops = true;
			SetThink(&CEnvModelCoop::RepeatSequenceThink);
			pev->nextthink = gpGlobals->time + 0.1f;
		}
	}
	
	// 
	void RepeatSequenceThink()
	{
		StudioFrameAdvance();
		if (m_fSequenceFinished)
		{	
		
			if (action_mode == 2)
			{
				m_fSequenceLoops = false;
				Use(NULL, NULL, USE_TOGGLE ,0.0); // instead of repeating switch off/on && activate
				pev->nextthink = gpGlobals->time + 0.01f;
				return;
			}
			
			if (action_mode == 1)
			{
				pev->frame = 0;
				m_fSequenceFinished = false;
				ResetSequenceInfo();
				pev->nextthink = gpGlobals->time + 0.01f;
			}
			else //action_mode = 0
			{
				//pev->nextthink = gpGlobals->time + 0.1f;
			}
		}
		else
		{
			pev->nextthink = gpGlobals->time + 0.01f;
		}
	}	
};

LINK_ENTITY_TO_CLASS(env_model_coop, CEnvModelCoop)