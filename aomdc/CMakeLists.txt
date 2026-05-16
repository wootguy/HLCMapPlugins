project(aomdc)

add_library(${PROJECT_NAME} SHARED
	env_hidehud.cpp
	env_model.cpp
	trigger_ripent.cpp
	
	monster_ghost.cpp
	monster_david.cpp
	wheelchair_monster.cpp
	zombie.cpp
	
	weapon_hammer.cpp
	weapon_spear.cpp
	
	item_flashlight.cpp
	
	aomdc.cpp
)

hlcoop_setup_plugin("plugins/maps/")