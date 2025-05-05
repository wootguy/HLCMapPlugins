# kmkz's utility scripts and entities
# https://github.com/kmkz272/Sven-Co-op-scripts

project(kmkz)

add_library(${PROJECT_NAME} SHARED
	env_warpball.cpp
)

hlcoop_setup_plugin("plugins/maps/")