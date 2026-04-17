project(crossedpaths)

add_library(${PROJECT_NAME} SHARED
	#asscart.cpp
	crossedpaths2.cpp
	#crossedpaths3.cpp
	#laseractivation.cpp
	#mbg.cpp
	messages.cpp
	#towerofhanoi.cpp
)

hlcoop_setup_plugin("plugins/maps/")