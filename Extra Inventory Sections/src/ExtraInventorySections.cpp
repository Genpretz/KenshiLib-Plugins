#include "LektorPushBack.h"

#include <Debug.h>
#include <core/Functions.h>
#include <kenshi/Character.h>
#include <kenshi/Enums.h>
#include <kenshi/GameData.h>
#include <kenshi/Inventory.h>
#include <kenshi/RootObjectFactory.h>
#include <kenshi/TitleScreen.h>

static void (*chooseMyClothing_orig)(lektor<GameData*>& gear, GameData* dataList, const std::string& listName, RaceData* race, bool noShoes) = 0;
static GameData* (*_chooseClothingItemFromList_orig)(GameData* dataList, const std::string& listName, AttachSlot slot, RaceData* race) = 0;
InventorySection* (*Inventory_getSectionOfType_orig)(Inventory* thisptr, AttachSlot type) = 0;
void (*Character_NV_init_orig)(Character* thisptr) = 0;
void (*BaseLayout_initialise_orig)(wraps::BaseLayout*, const std::string&, MyGUI::Widget*, bool, bool) = 0;

struct SectionInfo {
    const char* name;
    int width;
    int height;
    AttachSlot slot;
    bool equipCallbacks;
    int limit;
};

// This function checks for the existence of our extra inventory sections and creates them if they don't already exist.
void ensureExtraInventorySections(Inventory* inv)
{
    if (!inv)
    {
        ErrorLog("ensureExtraInventorySections: Inventory pointer missing or null");
        return;
    }

    // hardcoded section info for our extra inventory sections
    static const SectionInfo sections[] = {
        {"eyes_eyes",  3, 1, ATTACH_EYES,   true, 1},
        {"eyes_belts", 3, 1, ATTACH_BELT,   true, 1},
        {"eyes_hats",  3, 1, ATTACH_HAT,    true, 1},
        {"neck",       2, 2, ATTACH_NECK,   true, 1},
        {"gloves",     2, 2, ATTACH_GLOVES, true, 4}
    };

    const int count = (int)(sizeof(sections) / sizeof(sections[0]));
    for (int i = 0; i < count; ++i)
    {
        const SectionInfo& s = sections[i];
        InventorySection* section = inv->getSection(s.name);

        if (!section)
        {
            // store returned pointer so we can validate and correct sizes if necessary
            InventorySection* newSection = inv->_NV_initialiseNewSection(
                s.name, s.width, s.height,
                s.slot, s.equipCallbacks,
                false, true, s.limit
            );

            if (!newSection)
            {
                std::string sectionNameStr = "ensureExtraInventorySections: failed to create section " + std::string(s.name);
                ErrorLog(sectionNameStr.c_str());
                continue;
            }

            // If the created section does not match expected dims, resize it.
            // This avoids Array2d out-of-range access if constructor used swapped dims or other mismatch.
            if (newSection->width != s.width || newSection->height != s.height)
            {
                std::ostringstream oss;
                oss << "ensureExtraInventorySections: section '" << s.name
                    << "' created with unexpected size (got " << newSection->width << " x " << newSection->height
                    << ", expected " << s.width << " x " << s.height << "). Resizing.";
                ErrorLog(oss.str());
                inv->resizeSection(newSection, s.width, s.height, true);
            }
        }
    }
}

// We hook the Inventory::getSectionOfType function because the game uses that function to determine which inventory section to use for equipping items.
// Basically the game will try and equip items that won't fit in our new sections and this causes an issue with Array2d out-of-range errors and crashes.
static InventorySection* PreferVanillaEquipSection(Inventory* inv, AttachSlot type, InventorySection* current)
{
    if (!inv) return current;

    // - ATTACH_HAT must route to mygui widget "head"
    // - ATTACH_BELT must route to mygui widget "belt"
    if (type == ATTACH_HAT)
    {
        InventorySection* head = inv->getSection("head");
        if (head) return head;
    }
    else if (type == ATTACH_BELT)
    {
        InventorySection* belt = inv->getSection("belt");
        if (belt) return belt;
    }

    return current;
}


InventorySection* Inventory_getSectionOfType_Hook(Inventory* thisptr, AttachSlot type)
{
    InventorySection* sec = Inventory_getSectionOfType_orig(thisptr, type);
    return PreferVanillaEquipSection(thisptr, type, sec);
}

// We hook the Character::_NV_init function to ensure our extra inventory sections are created early enough to be available for all characters.
void Character_NV_init_hook(Character* thisptr)
{
    Character_NV_init_orig(thisptr);
    ensureExtraInventorySections(thisptr->inventory);
}

// We hook the chooseMyClothing function to add items for our extra inventory sections when the game spawns clothing for a character.
// The base game will only look for clothing items belonging to the ATTACH_HAT, ATTACH_BELT, ATTACH_BODY, ATTACH_LEGS, and ATTACH_SHIRT slots.
// By hooking this function, we can also look for items belonging to the ATTACH_EYES, ATTACH_GLOVES, and ATTACH_NECK slots and add those items from the character's clothing list as well.
static void chooseMyClothing_hook(lektor<GameData*>& gear, GameData* dataList, const std::string& listName, RaceData* race, bool noShoes)
{
    chooseMyClothing_orig(gear, dataList, listName, race, noShoes);

    if (listName != "clothing")
    {
        return;
    }
   
    static const AttachSlot extraSlots[] = {
        ATTACH_EYES, //eyes_eyes
        ATTACH_GLOVES, //gloves
        ATTACH_NECK, //neck
		//ATTACH_HAT, //eyes_hats
		//ATTACH_BELT //eyes_belts
    };

 // This game already chooses clothing for ATTACH_BELT and ATTACH_HAT slots from the "clothing" list, so we don't need to do anything special for those sections unless we would like the game to be able to populate both the original head/belt sections and our new ones at the same time, which would require some extra work.
 // If we want to populate both the original head/belt sections and our new ones, we would need to add some extra logic to determine which item goes in which section. If for example, you have a character template that has a chance of spawning with a helmets and a pair of glasses (both using ATTACH_HAT), 
 // when we "choose" our second ATTACH_HAT item, there's no gurantee it will fit in our new section in which case it will default to the "main" section. So instead of a character spawning with both a helmet and glasses, you might end up with a character that spawns with two helmets and an empty glasses section.

    const int count = (int)(sizeof(extraSlots) / sizeof(extraSlots[0]));
    for (int i = 0; i < count; ++i)
    {
        GameData* item = _chooseClothingItemFromList_orig(dataList, listName, extraSlots[i], race);

        if (item)
        {
            lektor_push_back(gear, item);
        }
    }
}

//We include this hook to ensure that we have a pointer to the original _chooseClothingItemFromList function, which we need to call from our chooseMyClothing_hook function.
static GameData* _chooseClothingItemFromList_hook(GameData* dataList, const std::string& listName, AttachSlot slot, RaceData* race)
{
    return _chooseClothingItemFromList_orig(dataList, listName, slot, race);
}

// There are a few different ways we can create the MyGUI widgets needed for our extra inventory sections, but this is the one that I found to be the most reliable.
// Other methods put your layout file at risk of being overwritten by the game or other mods (specifically UI mods) and that always leads to the game crashing.
// You can tell the user to place the mod in a specific load order position to avoid this, but that isn't ideal, especially with some users having 200+ mods.
void BaseLayout_initialise_Hook(wraps::BaseLayout* thisptr, const std::string& layout, MyGUI::Widget* parent, bool _throw, bool _createFakeWidgets)
{
    if (layout == "Kenshi_InventoryCharacterWindow.layout")
    {
        BaseLayout_initialise_orig(
            thisptr,
            "Custom_InventoryCharacterWindow.layout",
            parent, _throw, _createFakeWidgets
        );
        return;
    }

    BaseLayout_initialise_orig(thisptr, layout, parent, _throw, _createFakeWidgets);
}

__declspec(dllexport) void startPlugin()
{
	// Hook the Character::_NV_init function to ensure our extra inventory sections are created early enough for all characters.
    if (KenshiLib::SUCCESS != KenshiLib::AddHook(
        KenshiLib::GetRealAddress(&Character::_NV_init),
        &Character_NV_init_hook,
        &Character_NV_init_orig
    ))
    {
        ErrorLog("Failure hooking Character::_NV_init.");
    }

	// Hook the RootObjectFactory::chooseMyClothing and chooseMyClothingFromList functions to add items for our extra inventory sections when the game spawns clothing for a character.
    if (KenshiLib::SUCCESS != KenshiLib::AddHook(
        KenshiLib::GetRealAddress(&RootObjectFactory::chooseMyClothing),
        &chooseMyClothing_hook,
        &chooseMyClothing_orig
    ))
    {
        ErrorLog("Failure hooking RootObjectFactory::chooseMyClothing.");
    }

    if (KenshiLib::SUCCESS != KenshiLib::AddHook(
        KenshiLib::GetRealAddress(&RootObjectFactory::_chooseClothingItemFromList),
        &_chooseClothingItemFromList_hook,
        &_chooseClothingItemFromList_orig
    ))
    {
        ErrorLog("Failure hooking RootObjectFactory::_chooseClothingItemFromList.");
    }

	// Hook the BaseLayout::initialise function to replace the inventory character window layout with our custom version that has extra inventory sections.
    if (KenshiLib::SUCCESS != KenshiLib::AddHook(
        KenshiLib::GetRealAddress(&wraps::BaseLayout::initialise),
        &BaseLayout_initialise_Hook,
        &BaseLayout_initialise_orig
    ))
    {
        ErrorLog("Failure hooking wraps::BaseLayout::initialise.");
    }

    if (KenshiLib::SUCCESS != KenshiLib::AddHook(
        KenshiLib::GetRealAddress(&Inventory::getSectionOfType),
        &Inventory_getSectionOfType_Hook,
        &Inventory_getSectionOfType_orig
    ))
    {
        ErrorLog("[Extra Inventory Sections] Failure hooking Inventory::getSectionOfType.");
    }
}
