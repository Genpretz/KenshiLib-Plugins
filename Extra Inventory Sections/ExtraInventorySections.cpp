// ExtraInventorySections_Release.cpp
//
// Release build (minimal logging):
//   1) Ensures your extra inventory sections exist
//   2) Resizes them during Character::_NV_setupInventorySections (load-time path)
//   3) Prevents duplicated AttachSlot sections from hijacking equip routing by forcing
//      Inventory::getSectionOfType(ATTACH_HAT) -> "head" and ATTACH_BELT -> "belt"
//
// VS2010-safe.

#include <Debug.h>
#include <core/Functions.h>
#include <kenshi/Character.h>
#include <kenshi/Inventory.h>
#include <kenshi/GameSaveState.h>

#include <string>
#include <cstring> // strlen

#include <ogre/OgreMemoryAllocatorConfig.h> // Ogre::STLAllocator
#include <boost/unordered/unordered_map.hpp>
#include <boost/functional/hash.hpp>

// -----------------------------
// Section creation / resize logic
// -----------------------------
static void ensureExtraInventorySections(Inventory* inv, bool isLoading)
{
	if (!inv) return;

	struct SectionInfo {
		const char* name;
		int width;
		int height;
		AttachSlot slot;
		bool equipCallbacks;
		int limit;
	};

	// NOTE:
	// eyes_hats uses ATTACH_HAT, but equip routing is forced back to "head" in getSectionOfType hook.
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
			// Create section if missing.
			inv->_NV_initialiseNewSection(
				s.name,
				s.width,
				s.height,
				s.slot,
				s.equipCallbacks,
				false, // isContainerSlot
				true,  // enabled
				s.limit
			);
		}
		else if (isLoading)
		{
			// Resize on load-time path to keep save compatibility and keep dimensions consistent.
			// InventorySection::resize is protected; use Inventory::resizeSection (public).
			inv->resizeSection(section, s.width, s.height, false /*clearContent*/);
		}
	}
}

// -----------------------------
// Hooks: Character inventory lifecycle
// -----------------------------
bool (*Character_NV_setupInventorySections_orig)(Character* thisptr, GameSaveState* state) = 0;
void (*Character_NV_loadFromSerialise_orig)(Character* thisptr, GameSaveState* state) = 0;

bool Character_NV_setupInventorySections_Hook(Character* thisptr, GameSaveState* state)
{
	// Call original first to restore vanilla inventory sections.
	bool result = Character_NV_setupInventorySections_orig(thisptr, state);

	Inventory* inv = (thisptr ? thisptr->_NV_getInventory() : 0);
	if (inv)
	{
		// "isLoading" true here so we can resize existing sections as needed.
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
		// No resizing here to minimize mutation after load; creation-only is usually safest.
		ensureExtraInventorySections(inv, false /*isLoading*/);
	}
}

// -----------------------------
// Hook: Inventory routing (critical when multiple sections share AttachSlot)
// -----------------------------
InventorySection* (*Inventory_getSectionOfType_orig)(Inventory* thisptr, AttachSlot type) = 0;

static InventorySection* PreferVanillaEquipSection(Inventory* inv, AttachSlot type, InventorySection* current)
{
	if (!inv) return current;

	// Preserve canonical equip routing invariants:
	// - ATTACH_HAT must route to "head"
	// - ATTACH_BELT must route to "belt"
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

// -----------------------------
// Startup
// -----------------------------
__declspec(dllexport) void startPlugin()
{
	// Character lifecycle hooks
	if (KenshiLib::SUCCESS != KenshiLib::AddHook(
		KenshiLib::GetRealAddress(&Character::_NV_setupInventorySections),
		&Character_NV_setupInventorySections_Hook,
		&Character_NV_setupInventorySections_orig))
	{
		ErrorLog("[ExtraSlots] Failure hooking Character::_NV_setupInventorySections.");
	}

	if (KenshiLib::SUCCESS != KenshiLib::AddHook(
		KenshiLib::GetRealAddress(&Character::_NV_loadFromSerialise),
		&Character_NV_loadFromSerialise_Hook,
		&Character_NV_loadFromSerialise_orig))
	{
		ErrorLog("[ExtraSlots] Failure hooking Character::_NV_loadFromSerialise.");
	}

	// Inventory routing hook (prevents duplicated AttachSlot sections from hijacking equip routing)
	if (KenshiLib::SUCCESS != KenshiLib::AddHook(
		KenshiLib::GetRealAddress(&Inventory::getSectionOfType),
		&Inventory_getSectionOfType_Hook,
		&Inventory_getSectionOfType_orig))
	{
		ErrorLog("[ExtraSlots] Failure hooking Inventory::getSectionOfType.");
	}
}
