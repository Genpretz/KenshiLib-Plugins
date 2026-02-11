#include <Debug.h>
#include <core/Functions.h>
#include <kenshi/Character.h>
#include <kenshi/Inventory.h>
#include <mygui/MyGUI.h>
#include <mygui/MyGUI_Gui.h>

// -----------------------------
// Section creation / resize logic
// -----------------------------

static bool HasRequiredInventoryWidgets()
{
	MyGUI::Gui* gui = MyGUI::Gui::getInstancePtr();
	if (!gui)
		return false;

	static const char* requiredWidgets[] = {
		"eyes_eyes",
		"eyes_belts",
		"eyes_hats",
		"gloves",
		"neck"
	};

	const int count = sizeof(requiredWidgets) / sizeof(requiredWidgets[0]);
	for (int i = 0; i < count; ++i)
	{
		if (!gui->findWidget<MyGUI::Widget>(requiredWidgets[i], false))
			return false;
	}

	return true;
}


static bool g_uiConflictDetected = false;
static bool g_uiConflictLogged = false;

static void ensureExtraInventorySections(Inventory* inv, bool isLoading)
{
	if (!inv)
	{
		ErrorLog("[Extra Inventory Sections] ensureExtraInventorySections: inventory pointer missing");
		return;
	}

	if (!HasRequiredInventoryWidgets())
	{
		g_uiConflictDetected = true;

		if (!g_uiConflictLogged)
		{
			g_uiConflictLogged = true;
			ErrorLog("[Extra Inventory Sections] UI mod conflict detected: required widgets missing from Kenshi_InventoryCharacterWindow.layout"
			);
		}
		return; // Inventory Sections not created
	}

	struct SectionInfo {
		const char* name;
		int width;
		int height;
		AttachSlot slot;
		bool equipCallbacks;
		int limit;
	};

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
			InventorySection* newSection = inv->_NV_initialiseNewSection(
				s.name,
				s.width, // Section width
				s.height, // Section height
				s.slot, // Attach slot type
				s.equipCallbacks,
				false, // isContainerSlot
				true,  // enabled
				s.limit // # of items able to be equipped
			);
		}
		else if (isLoading)
		{
			// Resize on load and keep dimensions consistent. We do this because for some reason the game sometimes switches the width/height of sections during save/load?
			// It's more likely that I just don't know what I'm doing, but just in case...
			inv->resizeSection(section, s.width, s.height, false /*clearContent*/);
		}
	}
}

// Sometimes items equipped in the vanilla equip sections attempt to route to our new sections causing crashes so we make sure to prefer the vanilla sections here.
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

// -----------------------------
// Hooks
// -----------------------------
bool (*Character_NV_setupInventorySections_orig)(Character* thisptr, GameSaveState* state) = 0;
void (*Character_NV_loadFromSerialise_orig)(Character* thisptr, GameSaveState* state) = 0;
InventorySection* (*Inventory_getSectionOfType_orig)(Inventory* thisptr, AttachSlot type) = 0;

bool Character_NV_setupInventorySections_Hook(Character* thisptr, GameSaveState* state)
{
	// Call original function to setup vanilla inventory sections first.
	bool result = Character_NV_setupInventorySections_orig(thisptr, state);

	Inventory* inv = (thisptr ? thisptr->_NV_getInventory() : 0);
	if (inv)
	{
		// "isLoading" istrue here so we can resize existing sections as needed.
		ensureExtraInventorySections(inv, true /*isLoading*/);
	}

	return result;
}

void Character_NV_loadFromSerialise_Hook(Character* thisptr, GameSaveState* state)
{
	Character_NV_loadFromSerialise_orig(thisptr, state);

	Inventory* inv = (thisptr ? thisptr->_NV_getInventory() : 0);
	if (inv)
	{
		// After serialise load, ensure our sections exist.
		ensureExtraInventorySections(inv, false /*isLoading*/);
	}
}

InventorySection* Inventory_getSectionOfType_Hook(Inventory* thisptr, AttachSlot type)
{
	InventorySection* sec = Inventory_getSectionOfType_orig(thisptr, type);
	return PreferVanillaEquipSection(thisptr, type, sec);
}

// -----------------------------
// DLL Entry Point
// -----------------------------
__declspec(dllexport) void startPlugin()
{
	if (KenshiLib::SUCCESS != KenshiLib::AddHook(
		KenshiLib::GetRealAddress(&Character::_NV_setupInventorySections),
		&Character_NV_setupInventorySections_Hook,
		&Character_NV_setupInventorySections_orig
	))
	{
		ErrorLog("[Extra Inventory Sections] Failure hooking Character::_NV_setupInventorySections.");
	}

	if (KenshiLib::SUCCESS != KenshiLib::AddHook(
		KenshiLib::GetRealAddress(&Character::_NV_loadFromSerialise),
		&Character_NV_loadFromSerialise_Hook,
		&Character_NV_loadFromSerialise_orig
	))
	{
		ErrorLog("[Extra Inventory Sections] Failure hooking Character::_NV_loadFromSerialise.");
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