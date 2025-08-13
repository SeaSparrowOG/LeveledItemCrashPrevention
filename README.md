## Leveled List Crash Prevention
Simple SKSE bugfix that prevents AddForm from adding forms to leveled lists with >255 entries. This prevents crashes.

Supports both [Skyrim Special Edition](https://www.nexusmods.com/skyrimspecialedition/mods/129152), [Anniversary Edition](https://www.nexusmods.com/skyrimspecialedition/mods/129136) and [Skyrim VR](https://www.nexusmods.com/skyrimspecialedition/mods/156773).

## Building
### Requirements:
- CMake
- VCPKG
- Visual Studio (with desktop C++ development)
---
### Instructions:
```
git clone https://github.com/SeaSparrowOG/LeveledListCrashPrevention
cd LeveledListCrashPrevention
git submodule init
git submodule update --recursive

# For Skyrim Special Edition (1.5.97):
cmake --preset vs2022-windows-vcpkg 
cmake --build build --config Release

# For Skyrim Annivesary Edition:
cmake --preset vs2022-windows-vcpkg-ae
cmake --build build-ae --config Release

# For Skyrim VR:
cmake --preset vs2022-windows-vcpkg-vr
cmake --build build-vr --config Release
```
---
### Automatic deployment to MO2:
You can automatically deploy to MO2's mods folder by defining an [Environment Variable](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_environment_variables?view=powershell-7.4) named SKYRIM_MODS_FOLDER and pointing it to your MO2 mods folder. It will create a new mod with the appropriate name. After that, simply refresh MO2 and enable the mod.