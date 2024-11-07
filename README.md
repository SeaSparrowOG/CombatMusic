# Combat Music
Combat music is a revolutionary mod that allows mod authors and users to escape patch hell when it comes to adding new combat music to Skyrim.

Traditionally, adding combat music required you to edit **all other combat music** tracks in MUSCombat. This was done to achieve a level of "exclusivity", so you wouldn't listen to, for example, Skyrim Combat Music in the Shivering Isles. However, this can very quickly get out of hand.

If you had 3+ mods that added conditions, the patching was already unbearable. Another mod, [MusicTypeDistributor](https://www.nexusmods.com/skyrimspecialedition/mods/119571) by ThirdEyeSqueegee attempted to solve this issue, but didn't alleviate the need for patches. This is where this mod comes in.

By introducing a condition system, you can swap out what track is considered the MUSCombat track, and use that intelligently. Combat Music determins what the best combat music is at runtime and saves you the headache.

Additionally, you can also swap "location cleared" music the same way you can replace combat music.
## Developer Information
In order to begin developing with Combat Music, you should familiarize yourself with JSON. JSON is a human readable data exchange language. It is perfect for complex configurations, like the ones present in many of my frameworks.

If you are already familiar with JSON, the rest will be very easy to follow. Starting off with where the configuration files go: As with my previous frameworks, configuration files go in `Data/SKSE/Plugins/CombatMusic` and take the form of JSON files.

Here is a basic skeleton configuration you can use:
```json
{
  "combatMusic": [
    {
      "isCombatMusic": true,
      "newMusic": "YourMusicEditorID",
      "worldspaces": {
        "AND": false,
        "forms": [ "EditorID", "EditorID2", "..." ]
      },
      "cells": {
        "AND": false,
        "forms": [ "EditorID", "EditorID2", "..." ]
      },
      "locations": {
        "AND": false,
        "forms": [ "Modname.esp|0x123ABC", "..." ]
      },
      "locationKeywords": {
        "AND": false,
        "forms": [ "EditorID", "..." ]
      },
      "combatTarget": {
        "AND": false,
        "forms": [ "Modname.esp|0x123ABC", "..." ]
      },
      "combatTargetKeywords": {
        "AND": false,
        "forms": [ "EditorID", "..." ]
      }
    }
  ]
}
```
### Fields
- `isCombatMusic`
If true, this is a combat music swap. If false, it is a location cleared music swap.
- `newMusic`
The music that will play if this is the best fitting combat music to use.
- `AND`
A flag that will make the particular condition (worldspace, cell, location...) necessary to fulfill if set to true.
- `forms`
An array of strings that represent the game objects you wish to filter for. Most of the time, you will be able to just use EditorIDs, but you can always use "Formatted Strings". 

Formatted Strings are special strings that speficy a form in a particular plugin, like so: `Skyrim.esm|0xf`. This particular string represents Gold, and consists of two parts: `Skyrim.esm` (the mod's name with the file extension) and `0xf` (the formID of the object).
- `worldspaces`
Our first condition. If any of the filters in a condition are met, the condition evaluates to "true". Conditions can be "OR" or "AND", just like the creation kit. Worldspaces refers to the world the player is currently in.
- `cells`
Cells refer to the cell the player is in.
- `locations`
Locations refer to the location the player is in.
- `locationKeywords`
The current location (or one of its parents) needs to have at least one of those keywords.
- `combatTarget`
CombatTarget refers to the current target of the player. This condition requires formatted strings within the form array, but you can use EditorIDs if using PO3's Tweaks. If the combat target of the player is any of these actorbases, the condition evaluates to true.
- `combatTargetKeyword`
Similar to CombatTarget, but returns true if the combat target has any of these keywords.

### Examples
1. Play the DLC2MUSCombat track as the default combat track in either Tamriel, or the Arcanaeum in the college of Winterhold. If this cell is not in Tamriel, the music will still be replaced with this configuration!
```json
{
  "combatMusic": [
    {
      "musicType": "DLC2MUSCombatBoss",
      "worldspaces": {
        "AND": false,
        "forms": [ "Tamriel" ]
      },
      "cells": {
        "AND": false,
        "forms": [ "WinterholdCollegeArcanaeum" ]
      }
    }
  ]
}
```
2. Play the MorrowindDwemerMusic track in Dwemer Ruins, but only in Solstheim. In Skyrim, they will still play the default battle music.
```json
{
  "combatMusic": [
    {
      "musicType": "MorrowindDwemerMusic",
      "worldspaces": {
        "AND": true,
        "forms": [ "DLC2SolstheimWorld" ]
      },
      "locationKeyword": {
        "AND": true,
        "forms": [ "LocTypeDwarvenAutomatons" ]
      }
    }
  ]
}
```
### Intelligent Choices
The plugin doesn't just favor the first combat music it finds. Instead, it tries to find an appropriate one among all options. Thus, you need to understand the process.

The first thing to understand is that the plugin *prefers* to use conditions with actors specified. If a rule has a potential location, cell, and even worldspace it will ALWAYS lose to a rule that specifies an actor. These rules are internally called "High Priority".

If two high priority, or two or more low priority rules are in conflict, the one that has matched the most filters will win. So for example:
```json
{
  "combatMusic": [
    {
      "musicType": "TamrielicDwemerCombat",
      "worldspaces": {
        "AND": true,
        "forms": [ "Tamriel" ]
      },
      "locationKeyword": {
        "AND": true,
        "forms": [ "LocTypeDwarvenAutomatons" ]
      }
    }
  ]
}
``` 
will always win against this.
```
{
  "combatMusic": [
    {
      "musicType": "TamrielNewCombatMusic",
      "worldspaces": {
        "AND": true,
        "forms": [ "Tamriel" ]
      }
    }
  ]
}
```
Note that "OR" conditions only count as 1 point. If you have 10 unique OR conditions that are met, you will only get 1 point. If you have 1 AND condition and 1 OR condition met, you will get 2 points.

## Building
### Requirements:
- CMake
- VCPKG
- Visual Studio (with desktop C++ development)
---
### Instructions:
```
git clone https://github.com/SeaSparrowOG/CombatMusic
cd SKSE-Plugin-Template
git submodule init
git submodule update --recursive
cmake --preset vs2022-windows-vcpkg 
cmake --build Release --config Release
```
---
### Automatic deployment to MO2:
You can automatically deploy to MO2's mods folder by defining an [Environment Variable](https://learn.microsoft.com/en-us/powershell/module/microsoft.powershell.core/about/about_environment_variables?view=powershell-7.4) named SKYRIM_MODS_FOLDER and pointing it to your MO2 mods folder. It will create a new mod with the appropriate name. After that, simply refresh MO2 and enable the mod.
