# kmkz's utility scripts and entities
# https://github.com/kmkz272/Sven-Co-op-scripts

project(kmkz)

add_library(${PROJECT_NAME} SHARED
	main.cpp
	env_model_coop.cpp
	env_warpball.cpp
	func_global.cpp
)

hlcoop_setup_plugin("plugins/maps/")