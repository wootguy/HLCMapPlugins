project(doom2)

add_library(${PROJECT_NAME} SHARED
	doom2.h				doom2.cpp
	doom_utils.h		doom_utils.cpp
	CDoomSprite.h		CDoomSprite.cpp
	CDoomProjectile.h	CDoomProjectile.cpp
	CDoomMonster.h		CDoomMonster.cpp
	sound_nodes.h		sound_nodes.cpp
	projectiles.cpp
	monsters.cpp
	items.cpp
)

hlcoop_setup_plugin("plugins/maps/")