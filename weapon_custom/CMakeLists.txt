project(weapon_custom)

add_library(${PROJECT_NAME} SHARED
	weapon_custom.h				weapon_custom.cpp
	CWeaponCustomBase.h			CWeaponCustomBase.cpp
	WeaponSound.h				WeaponSound.cpp
	CWeaponCustomAmmo.h			CWeaponCustomAmmo.cpp
	CWeaponCustomSound.h		CWeaponCustomSound.cpp
	CWeaponCustomShoot.h		CWeaponCustomShoot.cpp
	CWeaponCustomEffect.h		CWeaponCustomEffect.cpp
	CWeaponCustomUserEffect.h	CWeaponCustomUserEffect.cpp
	CWeaponCustomConfig.h		CWeaponCustomConfig.cpp
	CWeaponCustomProjectile.cpp
	util_wc.h					util_wc.cpp
)

hlcoop_setup_plugin("plugins/maps/")