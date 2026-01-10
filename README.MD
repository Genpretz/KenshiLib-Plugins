# KenshiLib Plugins

This repository contains a small collection of mods for Kenshi written in C++ and utilizing KenshiLib and RE_Kenshi.

## Usage Requirements
  - RE_Kenshi v0.3.0 or Later
      - Links: [Nexus Mods](https://www.nexusmods.com/kenshi/mods/847) or [Github](https://github.com/BFrizzleFoShizzle/RE_Kenshi/releases)

## Available Plugins

### Go Prone
  - The first available plugin in this repository.  
  - Go Prone introduces a new method of stealth movement for the player character by allowing them to enter a prone position at will. Previously, the player could only do so in specific situations.
### Extra Inventory Sections
  - Adds at least 3 new inventory sections for characters. These being Eyes, Hands, and Neck inventory sections, corresponding to ATTACH_EYES, ATTACH_GLOVES, ATTACH_NECK.
  - Along with the ATTACH_EYES sections are two for compatibilty with other Kenshi mods and are compatible with eyewear that uses the ATTACH_BELT AND ATTACH_HAT slots.
  - Unfourtunately, There are some issues currently. Items in the new inventory sections weren't saving properly. Currently, only the Hands, Neck, and Eyes(Belt) slots seem to be functioning.
### Faction Healing
  - Made by Matrix Agent, Edited by Genpretz
  - It's supposed to force NPC's to heal characters depending on faction, but it needs some work.

More plugins will be added over time.

## Compilation Requirements
  - Boost v1.60.0
  - KenshiLib v0.1.0 or Later
  - Visual Studio 2010 Platform Toolset
  - Please check out this repo for more information and links: https://github.com/BFrizzleFoShizzle/KenshiLib_Examples_deps

## Credits and Acknowledgements

This project would not be possible without the foundational work done by the Kenshi reverse-engineering community.

- [BFrizzlrFoShizzle](https://github.com/BFrizzleFoShizzle) – Primary author and maintainer of KenshiLib and RE_Kenshi  

## Disclaimer

This project is not affiliated with or endorsed by Lo-Fi Games. Kenshi is the property of its respective owners.
