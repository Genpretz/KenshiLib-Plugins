#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "InventorySectionsConfig.h"
#include "InventoryWidgetsConfig.h"
#include "CreateInventoryWidgets.h"

#include <Debug.h>

#include <core/Functions.h>

#include <kenshi/Character.h>
#include <kenshi/Inventory.h>
#include <kenshi/gui/TitleScreen.h>

#include <vector>

// -----------------------------
// DLL Entry Point
// -----------------------------

// We need the DLL directory for loading our JSON config, so we get it here in DllMain and cache it.
static std::string g_dllDir;

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		char path[MAX_PATH];
		if (GetModuleFileNameA(hModule, path, MAX_PATH))
		{
			std::string full(path);
			size_t lastSlash = full.find_last_of("\\/");
			if (lastSlash != std::string::npos)
				g_dllDir = full.substr(0, lastSlash + 1);
		}
	}
	return TRUE;
}

// -----------------------------
// Inventory sections
// -----------------------------

static void EnsureExtraInventorySections(Inventory* inv, bool isLoading)
{
	if (!inv)
		return;

	static std::vector<SectionInfo> cachedSections;
	static bool loaded = false;

	if (!loaded)
	{
		if (!LoadFromFile(g_dllDir + "InventorySections.json", cachedSections))
		{
			ErrorLog("[Extra Inventory Sections] Failed to load JSON config.");
			return;
		}
		loaded = true;
	}

	const int count = (int)cachedSections.size();
	for (int i = 0; i < count; ++i)
	{
		const SectionInfo& s = cachedSections[i];

		InventorySection* section = inv->getSection(s.name.c_str());

		if (!section)
		{
			inv->_NV_initialiseNewSection(
				s.name.c_str(),
				s.width,
				s.height,
				s.slot,
				s.equipCallbacks,
				s.isContainerSlot,
				s.enabled,
				s.limit
			);
		}
		else if (isLoading)
		{
			inv->resizeSection(section, s.width, s.height, false);
		}
	}
}

// It seems like equipment in a vanilla inventory section on loading will first look for sections matching the associated AttachSlot.
// This means that when adding a new section with an attach slot that's already used by default, sometimes the game will try to place items from those sections into our new ones.
// If for example this new section we added is too small to fit the item, then it can cause a crash. So we need to make sure that any vanilla inventory sections are properly referenced.
// The game likely uses something like Inventory::getSectionOfType and since all vanilla inventory sections basically had unique attach slots this wasn't an issue previously.
static InventorySection* PreferVanillaEquipSection(Inventory* inv, AttachSlot type, InventorySection* current)
{
	if (!inv)
		return current;

	const char* vanillaName = NULL;
	switch (type)
	{
	case ATTACH_HAT:          vanillaName = "head";              break;
	case ATTACH_BELT:         vanillaName = "belt";              break;
	case ATTACH_BACKPACK:     vanillaName = "backpack_attach";   break;
	case ATTACH_BODY:         vanillaName = "armour";            break;
	case ATTACH_LEGS:         vanillaName = "legs";              break;
	case ATTACH_SHIRT:        vanillaName = "shirt";             break;
	case ATTACH_BOOTS:        vanillaName = "boots";             break;
	case (AttachSlot)0x32:    vanillaName = "left_arm_replace";  break;
	case (AttachSlot)0x33:    vanillaName = "right_arm_replace"; break;
	case (AttachSlot)0x34:    vanillaName = "left_leg_replace";  break;
	case (AttachSlot)0x35:    vanillaName = "right_leg_replace"; break;
	default:                  vanillaName = NULL;                break;
	}

	if (vanillaName)
	{
		InventorySection* sec = inv->getSection(vanillaName);
		if (sec)
			return sec;
	}

	return current;
}

// -----------------------------
// Hooks
// -----------------------------

bool (*Character_NV_setupInventorySections_orig)(Character* thisptr, GameSaveState* state) = nullptr;
void (*Character_NV_loadFromSerialise_orig)(Character* thisptr, GameSaveState* state) = nullptr;
InventorySection* (*Inventory_getSectionOfType_orig)(Inventory* thisptr, AttachSlot type) = nullptr;
void (*BaseLayout_initialise_orig)(wraps::BaseLayout* thisptr, const std::string& _layout, MyGUI::Widget* _parent, bool _throw, bool _createFakeWidgets) = nullptr;

static bool Character_NV_setupInventorySections_Hook(Character* thisptr, GameSaveState* state)
{
	bool result = Character_NV_setupInventorySections_orig(thisptr, state);

	Inventory* inv = (thisptr ? thisptr->_NV_getInventory() : 0);
	if (inv)
		EnsureExtraInventorySections(inv, true /*isLoading*/);

	return result;
}

static void Character_NV_loadFromSerialise_Hook(Character* thisptr, GameSaveState* state)
{
	Character_NV_loadFromSerialise_orig(thisptr, state);

	Inventory* inv = (thisptr ? thisptr->_NV_getInventory() : nullptr);
	if (inv)
		EnsureExtraInventorySections(inv, false /*isLoading*/);
}

static InventorySection* Inventory_getSectionOfType_Hook(Inventory* thisptr, AttachSlot type)
{
	InventorySection* sec = Inventory_getSectionOfType_orig(thisptr, type);
	return PreferVanillaEquipSection(thisptr, type, sec);
}

static void BaseLayout_initialise_Hook(
	wraps::BaseLayout* thisptr,
	const std::string& _layout,
	MyGUI::Widget* _parent,
	bool _throw,
	bool _createFakeWidgets)
{
	BaseLayout_initialise_orig(thisptr, _layout, _parent, _throw, _createFakeWidgets);

	if (_layout.find("Kenshi_InventoryCharacterWindow.layout") == std::string::npos)
		return;

	if (!thisptr->mMainWidget)
		return;

	static std::vector<WidgetInfo> cachedWidgets;
	static bool loaded = false;

	if (!loaded)
	{
		if (!LoadWidgetsFromFile(g_dllDir + "InventorySectionWidgets.json", cachedWidgets))
		{
			ErrorLog("[Extra Inventory Sections] Failed to load widget config.");
			return;
		}
		loaded = true;
	}

	MyGUI::Widget* root = thisptr->mMainWidget;
	if (!root)
	{
		return;
	}

	std::string prefix = MyGUI::utility::toString(thisptr, "_");
	CreateInventoryWidgets(root, prefix, cachedWidgets);
	return;
}

// -----------------------------
// Exported Functions
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

	if (KenshiLib::SUCCESS != KenshiLib::AddHook(
		KenshiLib::GetRealAddress(&wraps::BaseLayout::initialise),
		&BaseLayout_initialise_Hook,
		&BaseLayout_initialise_orig
	))
	{
		ErrorLog("[Extra Inventory Sections] Failure hooking wraps::BaseLayout::initialise.");
	}
}