project(crossedpaths)

add_library(${PROJECT_NAME} SHARED
	asscart.cpp
	crossedpaths.cpp
	laseractivation.cpp
	mbg.cpp
	messages.cpp
	towerofhanoi.cpp
)

hlcoop_setup_plugin("plugins/maps/")