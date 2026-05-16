project(poke646)

add_library(${PROJECT_NAME} SHARED
	ammo_nailclip.cpp
	ammo_nailround.cpp
	ammo_par21_clip.cpp
	ammo_par21_grenade.cpp
	weapon_cmlwbr.cpp
	weapon_heaterpipe.cpp
	nail.cpp
	poke646.cpp
)

hlcoop_setup_plugin("plugins/maps/")