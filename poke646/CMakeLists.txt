project(poke646)

add_library(${PROJECT_NAME} SHARED
	ammo_nailclip.cpp
	ammo_nailround.cpp
	ammo_par21_clip.cpp
	ammo_par21_grenade.cpp
	weapon_bradnailer.cpp
	weapon_cmlwbr.cpp
	weapon_heaterpipe.cpp
	weapon_nailgun.cpp
	weapon_par21.cpp
	weapon_sawedoff.cpp
	poke646.cpp
)

hlcoop_setup_plugin("plugins/maps/")