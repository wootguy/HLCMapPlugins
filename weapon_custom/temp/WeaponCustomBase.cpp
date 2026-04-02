#include "attack"

namespace WeaponCustom {

class WeaponCustomBase : public CBasePlayerWeapon
{
	float m_flNextAnimTime;
	int m_iShell;
	int	m_iSecondaryAmmo;
	int lastButton; // last player button state
	int lastSequence;
	weapon_custom* settings;
	
	WeaponState state;

	int active_fire = -1;
	
	bool primaryAlt = false;
	
	float nextShootUserEffect = 0; // prevent stacking user effects
	
	string v_model_override;
	string p_model_override;
	string w_model_override;
	int w_model_body_override = -1;
	
	bool used = false; // set to true if the weapon has just been +USEd
	
	bool shouldRespawn = false;
	float m_flCustomRespawnTime = 0.0f;
	
	// when you cheat && give yourself a weapon, you get some extra ammo with it. Enabling this before spawning the
	// weapon will prevent that (used in the rust maps to prevent ammo duplication)
	bool shouldBypassAmmoExtraction = false;
	
	bool KeyValue( const string&szKey, const string&szValue )
	{
		if (szKey == "m_flCustomRespawnTime") m_flCustomRespawnTime = atof(szValue);
		else return BaseClass.KeyValue( szKey, szValue );
		
		return true;
	}
	
	void Spawn()
	{
		if (settings  == NULL ) {
			*settings = cast<weapon_custom>( *custom_weapons[pev->classname] );
		}		
		
		Precache();
		SET_MODEL( self, settings.wpn_w_model );

		m_iDefaultAmmo = settings.default_ammo;
		if (m_iDefaultAmmo == -1)
			m_iDefaultAmmo = settings.clip_size();		
		if (settings.default_ammo2 == -1)
			settings.default_ammo2 = settings.clip_size2;	
		m_iClip = m_iDefaultAmmo;
		m_iClip2 = settings.default_ammo2;
		
		FallInit();
		SetThink( ThinkFunction( WeaponThink ) );
		
		m_bExclusiveHold = settings->pev->spawnflags & FL_WEP_EXCLUSIVE_HOLD != 0;
		
		int idleSeq = LookupActivity(ACT_IDLE);
		pev.sequence = idleSeq != -1 ? idleSeq : 0;
		ResetSequenceInfo();
		
		shouldRespawn = true; // flag for respawning
	}

	bool AddWeapon()
	{
		bool wasUsed = used;
		used = false;
		return pev.spawnflags & FL_USE_ONLY == 0 || wasUsed;
	}
	
	void Use(CBaseEntity* pActivator, CBaseEntity* pCaller, USE_TYPE useType, float flValue = 0.0f)
	{
		if (pActivator  == NULL  || pCaller  == NULL )
			return;
		if (pActivator == pCaller && pCaller.IsPlayer())
		{
			CBasePlayer* plr = (CBasePlayer*)(pCaller);
			if (*plr.HasNamedPlayerItem(pev->classname) == NULL)
			{
				used = true; // allow pickups for time frame
				
				// touching || Collect() now would cause a crash
				g_Scheduler.SetTimeout("delay_touch", 0.0f, EHANDLE(self), EHANDLE(pCaller));
			}
		}
	}
	
	void Touch( CBaseEntity* pOther )
	{
		CBasePlayer* plr = (CBasePlayer*)(pOther);
		bool oldHadWep = plr  && *plr.HasNamedPlayerItem(pev->classname) != NULL;
		
		BaseClass.Touch(pOther);
		
		bool nowHasWep = plr  && *plr.HasNamedPlayerItem(pev->classname) != NULL;
		if (!oldHadWep && nowHasWep && shouldRespawn && (pev.spawnflags & FL_DISABLE_RESPAWN) == 0) {
			g_Scheduler.SetTimeout("delay_respawn", m_flCustomRespawnTime, settings.weapon_classname, pev.origin, pev.angles);
		}
	}
	
	void Precache()
	{
		PrecacheCustomModels();
	}
	
	CBasePlayer* getPlayer()
	{
		CBaseEntity* e_plr = m_hPlayer;
		return (CBasePlayer*)(e_plr);
	}

	void RegenAmmo(int ammoType)
	{
		CBasePlayer* plr = getPlayer();
		if (plr  == NULL )
			return;
		int ammoLeft = plr.m_rgAmmo(ammoType);
		int maxAmmo = plr.GetMaxAmmo(ammoType);
		ammoLeft += settings.primary_regen_amt;
		
		if (ammoLeft < 0) 
			ammoLeft = 0;
		if (ammoLeft > maxAmmo) 
			ammoLeft = maxAmmo;
			
		plr.m_rgAmmo(ammoType, ammoLeft);
	}
	
	bool GetItemInfo( ItemInfo&info )
	{
		if (settings  == NULL ) {
			*settings = cast<weapon_custom>( *custom_weapons[pev->classname] );
		}
		
		// custom ammo only. Also why would you ever want inconsistent max ammo counts?
		info.iMaxAmmo1 	= 9999999;
		info.iMaxAmmo2 	= 9999999;
		
		info.iAmmo1Drop	= settings.primary_ammo_drop_amt;
		info.iAmmo2Drop	= settings.secondary_ammo_drop_amt;
		
		info.iMaxClip = 9999999; // just prevents dynamic clip sizes from working
		if (settings.clip_size() < 1)
			m_iClip = -1;
		if (settings.clip_size2 < 1)
			m_iClip2 = -1;	

		info.iSlot 		= settings.slot;
		info.iPosition 	= settings.slotPosition;
		info.iFlags 	= settings->pev->spawnflags & 0x1F;
		info.iWeight 	= settings.priority;

		// ammo regeneration
		if (settings.primary_regen_amt != 0 && state.lastPrimaryRegen < gpGlobals->time)
		{
			state.lastPrimaryRegen = gpGlobals->time + settings.primary_regen_time;
			RegenAmmo(m_iPrimaryAmmoType);
			
		}
		if (settings.secondary_regen_amt != 0 && state.lastSecondaryRegen < gpGlobals->time)
		{
			state.lastSecondaryRegen = gpGlobals->time + settings.secondary_regen_time;
			RegenAmmo(m_iPrimaryAmmoType);
		}
		
		if (settings->pev->spawnflags & FL_WEP_HIDE_SECONDARY_AMMO != 0 && settings.matchingAmmoTypes)
			m_iSecondaryAmmoType = m_iPrimaryAmmoType;
		
		return true;
	}
	
	bool AddToPlayer( CBasePlayer* pPlayer )
	{
		if( BaseClass.AddToPlayer( pPlayer ) )
		{
			MESSAGE_BEGIN message( MSG_ONE, gmsgWeapPickup, pPlayer->edict() );
				message.WRITE_LONG( m_iId );
			message.MESSAGE_END();
			
			// add to ammo if clip is not used (grenades, snarks, etc.)
			if (settings.clip_size() == 0)
			{
				int ammoType = m_iPrimaryAmmoType;
				if (ammoType != -1)
				{
					int ammoLeft = pPlayer.m_rgAmmo(ammoType);
					pPlayer.m_rgAmmo(ammoType, ammoLeft + m_iDefaultAmmo);
				}
			} else {
				int oldClip = m_iClip;
			
				if (!shouldBypassAmmoExtraction)
					ExtractAmmo(self); // fix for losing half ammo when touching/using a dropped weapon
				
				// Fix for weird bug that places half of the held ammo into the clip, if given a weapon with an empty clip.
				// If this is always done, then all given weapons will have empty clips.
				if (oldClip == 0) {				
					ExtractClipAmmo(self);
				}
				
				shouldBypassAmmoExtraction = false; // only do this once to prevent losing ammo after dropping a newly spawned weapon
			}
			
			
			
			return true;
		}
		return false;
	}
	
	bool CustomCanDeploy() 
	{
		bool bHasAmmo = false;
		CBasePlayer* plr = getPlayer();
		
		if ( m_iPrimaryAmmoType == -1 || pev->spawnflags & FL_WEP_SELECTONEMPTY != 0 || true)
		{
			// this weapon doesn't use ammo, can always deploy.
			return true;
		}

		if ( m_iPrimaryAmmoType != -1 )
		{
			bHasAmmo = bHasAmmo || (plr.m_rgAmmo(m_iPrimaryAmmoType) != 0);
		}
		if ( m_iSecondaryAmmoType != -1 )
		{
			bHasAmmo = bHasAmmo || (plr.m_rgAmmo(m_iSecondaryAmmoType) != 0);
		}
		if (m_iClip > 0)
		{
			bHasAmmo = true;
		}

		return bHasAmmo;
	}
	
	bool CustomDeploy( string szViewModel, string szWeaponModel, int iAnim, string szAnimExt, int skiplocal, int body )
	{
		if ( !CustomCanDeploy() )
			return false;

		CBasePlayer* plr = getPlayer();
		
		plr->pev->viewmodel = szViewModel;
		plr->pev->weaponmodel = szWeaponModel;
		plr.set_m_szAnimExtension(szAnimExt);
		
		SendWeaponAnim( iAnim, skiplocal, body );

		plr.m_flNextAttack = 0.5;
		m_flTimeWeaponIdle = 1.0;
		return true;
	}
	
	bool Deploy(bool skipDelay)
	{
		CBasePlayer* plr = getPlayer();
		*state.user = plr;
		*state.wep = self;
		*state.c_wep = this;
		
		if (settings->pev->spawnflags & FL_WEP_LASER_SIGHT != 0 && !primaryAlt)
		{
			ShowLaser();
			state.unhideLaserTime = gpGlobals->time + 0.5;
		}
		
		string v_mod = v_model_override.Length() > 0 ? v_model_override : settings.wpn_v_model;
		string p_mod = p_model_override.Length() > 0 ? p_model_override : settings.wpn_p_model;
		string w_mod = w_model_override.Length() > 0 ? w_model_override : settings.wpn_w_model; // todo: this
		
		// body not used until weapon dropped
		
		bool ret = CustomDeploy( GetV_Model( v_mod ), GetP_Model( p_mod ), 
								   settings.deploy_anim, settings.getPlayerAnimExt(), 0, w_body() );
		
		SET_MODEL( self, w_mod );
		
		if (!skipDelay)
		{
			m_flTimeWeaponIdle = WeaponTimeBase() + settings.deploy_time + 0.5f;		   
			state.deployTime = gpGlobals->time;
		}
		
		settings.deploy_snd.play(plr, CHAN_VOICE);
		
		// set max ammo counts for custom ammo
		vector<string>* keys = custom_ammos.getKeys();
		for (uint i = 0; i < keys.length(); i++)
		{
			weapon_custom_ammo* ammo = cast<weapon_custom_ammo*>(custom_ammos[keys[i]]);
			bool isCustomAmmo = ammo.ammo_type == -1;
			
			if (isCustomAmmo && ammo.custom_ammo_type == settings.primary_ammo_type)
				plr.SetMaxAmmo(ammo.custom_ammo_type, ammo.max_ammo);
		}
		
		// delay fixes speed not working on minigum weapon switch && initial spawn
		g_Scheduler.SetTimeout(*this, "applyPlayerSpeedMult", 0);
		return ret;
	}
	
	bool Deploy()
	{
		return Deploy(false);
	}
	
	int w_body()
	{
		return w_model_body_override >= 0 ? w_model_body_override : settings.wpn_w_model_body;
	}
	
	// may actually be sequential if the flag for that is enabled
	int getRandomShootAnim()
	{
		return getRandomAnim(state.active_opts.shoot_anims);
	}

	int getRandomMeleeAnim()
	{
		return getRandomAnim(state.active_opts.melee_anims);
	}

	int getRandomAnim(const vector<string>& anims)
	{
		if (anims.length() == 0)
			return 0;

		if (true)
			state.c_wep.lastSequence = (state.c_wep.lastSequence+1) % anims.length();
		else
			state.c_wep.lastSequence = RANDOM_LONG(0, anims.length()-1);
		//return LookupSequence(state.active_opts.shoot_anims[state.c_wep.lastSequence]);  // I wish this worked :<
		return atoi( anims[state.c_wep.lastSequence] );
	}
	
	void applyPlayerSpeedMult()
	{
		CBasePlayer* plr = getPlayer();
		if (!plr.m_hActiveItem.IsValid() || string(plr.m_hActiveItem.GetEntity()->pev->classname) != string(pev.classname))
			return;
		
		float mult = settings.movespeed;
		if (state.windingUp && !state.windingDown && !state.windupShooting)
			mult *= state.active_opts.windup_movespeed;
		if (state.windupShooting)
			mult *= state.active_opts.windup_shoot_movespeed;
		
		plr.SetMaxSpeedOverride(int(plr.GetMaxSpeed()*mult));
		
		if (settings->pev->spawnflags & FL_WEP_NO_JUMP != 0)
			plr->pev->fuser4 = 1;
	}
	
	void Kill()
	{
		CBasePlayer* plr = getPlayer();
		shouldRespawn = false;
		plr.RemovePlayerItem(self);
	}
	
	void Holster(int iSkipLocal = 0) 
	{
		// Cleanup beams, windups, etc.
		CBasePlayer* plr = getPlayer();
		
		if (state.hook_ent)
		{
			CBaseEntity* hookEnt = state.hook_ent;
			WeaponCustomProjectile* hookEnt_c = cast<WeaponCustomProjectile*>(CastToScriptClass(hookEnt));
			hookEnt_c.uninstall_steam_and_kill_yourself();
		}
		if (state.hook_beam)
			UTIL_Remove( state.hook_beam );
		state.hook_beam = NULL;
		
		HideLaser();
		if (m_fInZoom)
			primaryAlt = false;
		CancelZoom();
		CancelBeam(state);
		
		state.reloading = 0;
		state.reloading2 = 0;
		
		state.windingUp = false;
		state.windupLoopEntered = false;
		state.windupSoundActive = false;
		state.windingDown = false;
		state.windupFinished = false;
		state.windupAmmoUsed = 0;
		if (state.active_opts )
		{
			state.active_opts.windup_snd.stop(plr, CHAN_VOICE);
			state.active_opts.wind_down_snd.stop(plr, CHAN_VOICE);
			state.active_opts.windup_loop_snd.stop(plr, CHAN_VOICE);
			state.active_opts.hook_snd.stop(plr, CHAN_VOICE);
			state.active_opts.hook_snd2.stop(plr, CHAN_VOICE);
		}
		
		for (uint i = 0; i < state.ubeams.length(); i++)
			UTIL_Remove(state.ubeams[i]);
		
		plr.SetMaxSpeedOverride(-1);
		if (settings->pev->spawnflags & FL_WEP_NO_JUMP != 0)
			plr->pev->fuser4 = 0;
	}
	
	float WeaponTimeBase()
	{
		return gpGlobals->time; //g_WeaponFuncs.WeaponTimeBase();
	}
	
	void WeaponThink()
	{	 
		AttackThink(state);
	}
	
	// returns true if a windup was started
	bool DoWindup()
	{
		CBasePlayer* plr = getPlayer();
		if (state.active_opts.windup_time > 0 && !state.windingUp)
		{
			state.windingUp = true;
			state.windupLoopEntered = false;
			state.windingDown = false;
			state.windupFinished = false;
			state.windupHeld = true;
			state.lastWindupHeld = gpGlobals->time;
			state.windupSoundActive = false;
			state.windupOvercharged = false;
			state.windupMultiplier = 1.0f;
			state.windupKickbackMultiplier = 1.0f;
			state.windupStart = gpGlobals->time;
			pev->nextthink = gpGlobals->time;
			
			applyPlayerSpeedMult();
			
			if (state.active_opts.windup_cost > 0)
			{
				state.windupAmmoUsed = 1;
				DepleteAmmo(state, 1); // don't let user get away with free shots
			}
			
			EHANDLE h_plr = plr;
			EHANDLE h_wep = cast<CBaseEntity*>(self);
			custom_user_effect(h_plr, h_wep, *state.active_opts.user_effect4);
			
			if (settings.player_anims == ANIM_REF_CROWBAR)
			{
				// Manually set wrench windup animation
				plr.m_Activity = ACT_RELOAD;
				plr->pev->frame = 0;
				plr->pev->sequence = 25;
				plr.ResetSequenceInfo();
				//plr->pev->framerate = 0.5f;
			}
			if (settings.player_anims == ANIM_REF_GREN)
			{
				// Manually set wrench windup animation
				plr.m_Activity = ACT_RELOAD;
				plr->pev->frame = 0;
				plr->pev->sequence = 33;
				plr.ResetSequenceInfo();
				//plr->pev->framerate = 0.5f;
			}
			
			return true;
		}
		return false;
	}
	
	void ShowLaser()
	{
		CBasePlayer* plr = getPlayer();
		if (!state.laser_spr)
		{
			CSprite* dot = CSprite::SpriteCreate( settings.laser_sprite, plr->pev->origin, true, 10 );
			dot->pev->rendermode = kRenderGlow;
			dot->pev->renderamt = settings.laser_sprite_color.a;
			dot->pev->rendercolor = settings.laser_sprite_color.getRGB();
			dot->pev->renderfx = kRenderFxNoDissipation;
			dot->pev->movetype = MOVETYPE_NONE;
			dot->pev->scale = settings.laser_sprite_scale;
			state.laser_spr = dot;
			pev->nextthink = gpGlobals->time;
		} 
	}
	
	void HideLaser()
	{
		if (state.laser_spr)
		{
			CBaseEntity* ent = state.laser_spr;
			UTIL_Remove( ent );
		}
	}
	
	float GetNextAttack()
	{
		if (active_fire == 0) return m_flNextPrimaryAttack;
		if (active_fire == 1) return m_flNextSecondaryAttack;
		if (active_fire == 2) return m_flNextTertiaryAttack;
		return 0;
	}
	
	void SetNextAttack(float time)
	{
		if (active_fire == 0) m_flNextPrimaryAttack = time;
		if (active_fire == 1) m_flNextSecondaryAttack = time;
		if (active_fire == 2) m_flNextTertiaryAttack = time;
	}
	
	void CancelZoom()
	{
		SetFOV(0);
		m_fInZoom = false;
		if (settings.player_anims == ANIM_REF_BOW)
			getPlayer().set_m_szAnimExtension("bow");
	}
	
	void TogglePrimaryFire(int mode)
	{
		if (mode == PRIMARY_NO_CHANGE)
			return;
			
		if (mode == PRIMARY_FIRE && !primaryAlt || mode == PRIMARY_ALT_FIRE && primaryAlt)
			return;

		primaryAlt = !primaryAlt;
		
		weapon_custom_shoot* p_opts = *settings.fire_settings[0];
		weapon_custom_shoot* p_alt_opts = *settings.alt_fire_settings[0];
		
		weapon_custom_shoot* next_p_opts = primaryAlt ? *p_alt_opts : *p_opts;
		
		EHANDLE h_plr = getPlayer();
		EHANDLE h_wep = cast<CBaseEntity*>(self);
		custom_user_effect(h_plr, h_wep, *next_p_opts.user_effect5);
		
		state.nextActionTime = state.nextShootTime = WeaponTimeBase() + next_p_opts.toggle_cooldown;
	}
	
	void CommonAttack(int attackNum)
	{
		CBasePlayer* plr = getPlayer();
		int next_fire = attackNum;
		weapon_custom_shoot* next_opts = *settings.fire_settings[next_fire];
		weapon_custom_shoot* alt_opts = *settings.alt_fire_settings[next_fire];
		
		state.windupOnly = false;
		
		int fireAct = next_fire == 1 ? settings.secondary_action : settings.tertiary_action;
		if (fireAct != FIRE_ACT_SHOOT)
		{
			if (state.nextActionTime > gpGlobals->time)
				return;
			if (fireAct == FIRE_ACT_LASER)
			{
				if (!state.laser_spr)
					ShowLaser();
				else
					HideLaser();
			}
			if (fireAct == FIRE_ACT_ZOOM)
			{ 
				SetFOV(primaryAlt ? 0 : settings.zoom_fov);
				m_fInZoom = !m_fInZoom;
				if (settings.player_anims == ANIM_REF_BOW)
					plr.set_m_szAnimExtension(m_fInZoom ? "bowscope" : "bow");
				if (settings.player_anims == ANIM_REF_SNIPER)
					plr.set_m_szAnimExtension(m_fInZoom ? "sniperscope" : "sniper");
				
			}
			
			
			if (fireAct == FIRE_ACT_WINDUP)
			{
				next_fire = 0;
				*next_opts = *settings.fire_settings[next_fire];
				*alt_opts = *settings.alt_fire_settings[next_fire];
				state.windupOnly = true;
			}
			else
			{
				TogglePrimaryFire(PRIMARY_TOGGLE);
				return;
			}
			
		}
		
		if (next_fire == 0 && primaryAlt && settings.primary_alt_fire.Length() > 0)
			*next_opts = *alt_opts;
	
		if (next_opts.pev  == NULL  || !CanStartAttack(state, next_opts))
			return;
			
		*state.active_opts = next_opts;
		active_fire = next_fire;
		state.active_ammo_type = -1;
		if (next_fire == 0) 
		{
			state.active_ammo_type = m_iPrimaryAmmoType;
		} 
		else if (next_fire == 1)
		{
			state.active_ammo_type = m_iSecondaryAmmoType;
		}
		else if (next_fire == 2)
		{
			if (settings.tertiary_ammo_type == TAMMO_SAME_AS_PRIMARY)
				state.active_ammo_type = m_iPrimaryAmmoType;
			if (settings.tertiary_ammo_type == TAMMO_SAME_AS_SECONDARY) 
				state.active_ammo_type = m_iSecondaryAmmoType;
		}
		
		if (DoWindup())
			return;
		
		DoAttack(state);
	}
	
	void PrimaryAttack()   { CommonAttack(0); }
	void SecondaryAttack() { CommonAttack(1); }
	void TertiaryAttack()  { CommonAttack(2); }
	
	// Same as DefaultReload except it doesn't break when changing clip size mid-game
	bool CustomReload(int reloadAnim, float reloadTime, bool reloadingSecondary)
	{
		CBasePlayer* plr = getPlayer();
		
		int ammoType = reloadingSecondary ? m_iSecondaryAmmoType : m_iPrimaryAmmoType;
		int ammoLeft = plr.m_rgAmmo(ammoType);
		if (ammoLeft <= 0 || 
			(m_iClip == settings.clip_size() && !reloadingSecondary) or
			(m_iClip2 == settings.clip_size2 && reloadingSecondary))
			return false;
		
		SendWeaponAnim( reloadAnim, 0, w_body() );
		state.reloadFinishTime = gpGlobals->time + reloadTime;
		if (reloadingSecondary)
			state.reloading2 = -1;
		else
			state.reloading = -1;
		pev->nextthink = gpGlobals->time;
			
		return true;
	}
	
	void Reload()
	{
		bool reloadingSecondary = state.reloadSecondary;
		state.reloadSecondary = false;
		int reload_mode = reloadingSecondary ? settings.reload_mode2 : settings.reload_mode;
		int clip_size = reloadingSecondary ? settings.clip_size2 : settings.clip_size();
		int clip = reloadingSecondary ? m_iClip2 : m_iClip;
		
		CBasePlayer* plr = getPlayer();
		if (clip_size == 0)
			return;
		if (!cooldownFinished(state) || state.reloading != 0 || state.reloading2 != 0)
			return;
		if (state.reloadFinishTime > gpGlobals->time)
			return;
		if (state.liveProjectiles > 0 && state.active_opts.projectile.follow_mode == FOLLOW_CROSSHAIRS)
			return; // don't reload if we're controlling a projectile
			
		if (state.liveProjectiles > 0 && settings->pev->spawnflags & FL_WEP_WAIT_FOR_PROJECTILES != 0)
			return; // don't reload if user wants to wait for projectile deaths
		
		if ((reload_mode == RELOAD_STAGED || reload_mode == RELOAD_STAGED_RESPONSIVE) && 
			clip < clip_size && AmmoLeft(state, state.active_ammo_type) >= settings.reload_ammo_amt)
		{
			SendWeaponAnim( settings.reload_start_anim, 0, w_body() );
			state.reloading = 1;
			state.nextReload = WeaponTimeBase() + settings.reload_start_time;
			state.nextShootTime = state.nextReload;
			pev->nextthink = gpGlobals->time;
			settings.reload_start_snd.play(plr, CHAN_VOICE);
			CancelZoom();
			state.windupHeld = false;
			return;
		}
		
		bool emptyReload = EmptyShoot(state);
		int reloadAnim = settings.reload_empty_anim;
		if (reloadAnim < 0 || !emptyReload)
			reloadAnim = settings.reload_anim;
		if (reloadingSecondary)
			reloadAnim = settings.reload_anim2;
			
		bool emptyReloadEffect = emptyReload && settings.user_effect2 ;
		float reload_time = settings.getReloadTime(emptyReloadEffect, reloadingSecondary);
			
		bool reloaded = CustomReload(reloadAnim, reload_time, reloadingSecondary);
		
		if (reloaded)
		{
			if (reload_mode == RELOAD_EFFECT_CHAIN)
			{
				EHANDLE h_plr = plr;
				EHANDLE h_wep = cast<CBaseEntity*>(self);
				weapon_custom_user_effect* ef = emptyReloadEffect ? *settings.user_effect2 : *settings.user_effect1;
				if (reloadingSecondary)
					*ef = *settings.user_effect_r2;
				custom_user_effect(h_plr, h_wep, ef, false);
			}
		
			CancelZoom();
			if (reloadingSecondary)
				settings.reload_snd2.play(plr, CHAN_VOICE);
			else
				settings.reload_snd.play(plr, CHAN_VOICE);
			state.unhideLaserTime = WeaponTimeBase() + reload_time;
		}
		
		if (settings.player_anims == ANIM_REF_UZIS)
		{
			// Only reload the right uzi since dual wielding doesn't work yet.
			plr.m_Activity = ACT_RELOAD;
			plr->pev->frame = 0;
			plr->pev->sequence = 135;
			plr.ResetSequenceInfo();
			//plr->pev->framerate = 0.5f;
		}
		else
			BaseClass.Reload();
	}

	void WeaponIdle()
	{
		if (state.beam_active && state.beamStartTime + state.minBeamTime < gpGlobals->time)
		{
			CancelBeam(state);
		}
		
		if (state.nextCooldownEffect != 0 && state.nextCooldownEffect < gpGlobals->time)
		{
			state.nextCooldownEffect = 0;
			
			EHANDLE h_plr = getPlayer();
			EHANDLE h_wep = cast<CBaseEntity*>(self);
			custom_user_effect(h_plr, h_wep, *state.active_opts.user_effect3, true);
		}
		
		//println("FRAMERATE: " + pev->animtime);
		//pev->framerate = 500;
		//float wow = StudioFrameAdvance(0.0f);
		
		ResetEmptySound();
		
		if( m_flTimeWeaponIdle > WeaponTimeBase() || state.windingUp || state.reloading != 0 || state.reloading2 != 0 || state.nextActionTime > gpGlobals->time)
			return;

		if (settings.idle_time > 0) {
			SendWeaponAnim( settings.getRandomIdleAnim(), 0, w_body() );
			m_flTimeWeaponIdle = WeaponTimeBase() + settings.idle_time; // how long till we do this again.
		}
	}
}

class AmmoCustomBase : ScriptBasePlayerAmmoEntity
{
	weapon_custom_ammo* settings;
	
	void Spawn()
	{ 
		if (settings  == NULL ) {
			*settings = cast<weapon_custom_ammo>( *custom_ammos[pev->classname] );
		}
		
		Precache();

		if( !SetupModel() )
		{
			SET_MODEL( self, settings.w_model );
		}
		else	//Custom model
			SET_MODEL( self, pev->model );

		BaseClass.Spawn();
	}
	void Precache()
	{
		BaseClass.Precache();
	}
	bool AddAmmo( CBaseEntity* pOther ) 
	{
		if (pOther->pev->classname != "player")
			return false;
		CBasePlayer* plr = (CBasePlayer*)(pOther);
			
		int type = settings.ammo_type;
		string ammo_type = type < 0 ? settings.custom_ammo_type : g_ammo_types[type];
		
		// I don't like that you have to code a max ammo in each weapon. So I'm doing the math here.
		int should_give = settings.give_ammo;
		int total_ammo = plr.m_rgAmmo(g_PlayerFuncs.GetAmmoIndex(ammo_type));
		if (total_ammo >= settings.max_ammo)
			return false;
		else
			should_give = Math.min(settings.max_ammo - total_ammo, settings.give_ammo);
		
		int ret = pOther.GiveAmmo( should_give, ammo_type, settings.max_ammo );
		if (ret != -1)
		{
			settings.pickup_snd.play(self, CHAN_ITEM);
			return true;
		}
		return false;
	}
}

void delay_respawn(string classname, Vector pos, Vector angles) {
	CBaseEntity* ent = CBaseEntity::Create(classname, pos, angles, false); 
	EMIT_SOUND_DYN( ent->edict(), CHAN_ITEM, "items/suitchargeok1.wav", 1.0, ATTN_NORM, 0, 150 );
	WeaponCustomBase* wep = cast<WeaponCustomBase*>(CastToScriptClass(ent));
	wep.shouldRespawn = true; // respawn this one, too
}

}