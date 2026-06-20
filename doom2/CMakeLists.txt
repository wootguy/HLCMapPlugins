project(doom2)

add_library(${PROJECT_NAME} SHARED
	doom2.h				doom2.cpp
	doom_utils.h		doom_utils.cpp
	CDoomSprite.h		CDoomSprite.cpp
	CDoomProjectile.h	CDoomProjectile.cpp
	projectiles.cpp
)

hlcoop_setup_plugin("plugins/maps/")