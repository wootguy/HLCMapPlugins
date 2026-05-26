project(sweet_hl)

add_library(${PROJECT_NAME} SHARED
	main.cpp
	plasmaball.cpp
)

hlcoop_setup_plugin("plugins/maps/")