#include <Debug.h>
#include <core/Functions.h>
#include <kenshi/Character.h>
#include <kenshi/Inventory.h>

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

// -----------------------------
// Hooks:
// -----------------------------
bool (*Character_NV_setupInventorySections_orig)(Character* thisptr, GameSaveState* state) = 0;
void (*Character_NV_loadFromSerialise_orig)(Character* thisptr, GameSaveState* state) = 0;
InventorySection* (*Inventory_getSectionOfType_orig)(Inventory* thisptr, AttachSlot type) = 0;

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
		ensureExtraInventorySections(inv, false /*isLoading*/);
	}
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
	if (KenshiLib::SUCCESS != KenshiLib::AddHook(
		KenshiLib::GetRealAddress(&Character::_NV_setupInventorySections),
		&Character_NV_setupInventorySections_Hook,
		&Character_NV_setupInventorySections_orig))
	{
		ErrorLog("[Extra Inventory Sections] Failure hooking Character::_NV_setupInventorySections.");
	}

	if (KenshiLib::SUCCESS != KenshiLib::AddHook(
		KenshiLib::GetRealAddress(&Character::_NV_loadFromSerialise),
		&Character_NV_loadFromSerialise_Hook,
		&Character_NV_loadFromSerialise_orig))
	{
		ErrorLog("[Extra Inventory Sections] Failure hooking Character::_NV_loadFromSerialise.");
	}

	if (KenshiLib::SUCCESS != KenshiLib::AddHook(
		KenshiLib::GetRealAddress(&Inventory::getSectionOfType),
		&Inventory_getSectionOfType_Hook,
		&Inventory_getSectionOfType_orig))
	{
		ErrorLog("[Extra Inventory Sections] Failure hooking Inventory::getSectionOfType.");
	}
}
