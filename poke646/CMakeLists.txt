project(poke646)

add_library(${PROJECT_NAME} SHARED
	weapon_cmlwbr.cpp
	weapon_heaterpipe.cpp
	nail.cpp
	poke646.cpp
)

hlcoop_setup_plugin("plugins/maps/")