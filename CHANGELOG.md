# 1.10.1
* Fix iOS and Mac issues with the previous update

# 1.10.0
* Fix reloading SFX effect files when applying
* Remove fallback sprites (they are in Geode now)
* Add API call to open pack select popup
* Add spritesheet stacking (thanks @Cosmella-v)

This means texture pack creators can now only include changed sprites in their respective spritesheets and the rest will be loaded from the packs underneath. It is also convenient for people using outdated packs, as missing textures will be loaded from GD directly.

# 1.9.0
* Geode v5 support

# 1.8.2
* Simple rebuild for iOS JIT-less support

# 1.8.1
* Show tutorial for new button location

# 1.8.0
* Redesign the entire pack selection UI, courtesy of @Alphalaneous
  * Thanks alphalaneous!
* Textures button now moved to the vanilla options menu

# 1.7.0
* Add an optional API for mods to fetch info about loaded texture packs

# 1.6.1
* Fix music not stopping when reloading packs
* Fix crash caused by texture packs with invalid filenames on Windows

# 1.6.0
* Use fallback on missing sprites

# 1.5.0
* Add support for loading packs directly from zip or apk files

