project(insurgency)

add_library(${PROJECT_NAME} SHARED
	insurgency.cpp
	proj_ins2gl.cpp
	explosive_base.cpp
)

hlcoop_setup_plugin("plugins/maps/")