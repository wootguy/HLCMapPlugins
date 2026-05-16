project(io)

add_library(${PROJECT_NAME} SHARED
	io.cpp
	weapon_dualglock.cpp
	weapon_p228sil.cpp
)

hlcoop_setup_plugin("plugins/maps/")