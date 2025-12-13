project(aomdc)

add_library(${PROJECT_NAME} SHARED
	env_hidehud.cpp
	env_model.cpp
	trigger_ripent.cpp
	
	monster_ghost.cpp
	monster_david.cpp
	wheelchair_monster.cpp
	zombie.cpp
	
	weapon_aom_knife.cpp
	weapon_aom_shotgun.cpp
	weapon_axe.cpp
	weapon_beretta.cpp
	weapon_deagle.cpp
	weapon_glock.cpp
	weapon_hammer.cpp
	weapon_mp5k.cpp
	weapon_p228.cpp
	weapon_revolver.cpp
	weapon_spear.cpp
	weapon_uzi.cpp
	
	item_flashlight.cpp
	
	aomdc.cpp
)

hlcoop_setup_plugin("plugins/maps/")