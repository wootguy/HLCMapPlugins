project(io)

add_library(${PROJECT_NAME} SHARED
	io.cpp
	weapon_dualglock.cpp
)

hlcoop_setup_plugin("plugins/maps/")