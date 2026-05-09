# KenshiLib Plugins

This repository contains a small collection of mods for Kenshi written in C++ and utilizing KenshiLib and RE_Kenshi.
  - RE_Kenshi takes care of loading the plugins and KenshiLib is the api used to interact with Kenshi's internal functions.

## Usage Requirements
  - RE_Kenshi v0.3.0 or Later
      - Links: [Nexus Mods](https://www.nexusmods.com/kenshi/mods/847) or [Github](https://github.com/BFrizzleFoShizzle/RE_Kenshi/releases)

## Available Plugins

### Go Prone
  - The first available plugin in this repository.  
  - Go Prone introduces a new method of stealth movement for the player character by allowing them to enter a prone position at will. Previously, the player could only do so in specific situations.
### Extra Inventory Sections
  - Adds at least 3 new inventory sections for characters. These being Eyes, Hands, and Neck inventory sections, corresponding to ATTACH_EYES, ATTACH_GLOVES, ATTACH_NECK.
  - Along with the ATTACH_EYES sections are two more sections that are compatible with eyewear that uses the ATTACH_BELT AND ATTACH_HAT slots.

More plugins will be added over time.

## Compilation
#### Dependencies:
  - [KenshiLib](https://github.com/BFrizzleFoShizzle/KenshiLib)
  - Visual Studio 2010 SP1 (v100 Platform Toolset)
        - Downloads available on the Internet Archive
#### Recommended:
  - [KenshiLib_Examples_deps](https://github.com/BFrizzleFoShizzle/KenshiLib_Examples_deps)
      - This repository includes Boost and KenshiLib as well as a script that sets environment variables associated with the two and used by the project. I'd highly suggest using it when attempting to compile this repo for yourself.

## Credits and Acknowledgements
This project would not be possible without the foundational work done by the Kenshi reverse-engineering community.

- [BFrizzleFoShizzle](https://github.com/BFrizzleFoShizzle) – Primary author and maintainer of KenshiLib and RE_Kenshi  

## Disclaimer
This project is not affiliated with or endorsed by Lo-Fi Games. Kenshi is the property of its respective owners.
