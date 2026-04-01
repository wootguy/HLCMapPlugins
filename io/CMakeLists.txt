project(io)

add_library(${PROJECT_NAME} SHARED
	io.cpp
	weapon_colt.cpp
	weapon_dualglock.cpp
	weapon_fiveseven.cpp
	weapon_p228sil.cpp
	weapon_sawedoff.cpp
)

hlcoop_setup_plugin("plugins/maps/")