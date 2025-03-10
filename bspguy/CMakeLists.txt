project(bspguy)

add_library(${PROJECT_NAME} SHARED
	bspguy.cpp
	bspguy_equip.cpp
)

hlcoop_setup_plugin("plugins/maps/")