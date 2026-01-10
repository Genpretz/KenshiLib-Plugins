// NeckAndGlovesTestPlugin.cpp
// VS2010-safe. Ensures NECK + GLOVES sections exist early enough to restore items from saves.

#include <Debug.h>
#include <core/Functions.h>
#include <kenshi/Character.h>
#include <kenshi/Inventory.h>

// ---------- Create ONLY neck + gloves ----------
static void ensureExtraInventorySections(Inventory* inv)
{
	if (!inv) return;

	if (!inv->getSection("eyes_belts"))
	{
		inv->_NV_initialiseNewSection(
			"eyes_belts",
			3, 1,
			ATTACH_BELT,
			true, false, true,
			1);
	}

	if (!inv->getSection("neck"))
	{
		inv->_NV_initialiseNewSection(
			"neck",
			2, 2,
			ATTACH_NECK,
			true, false, true,
			1);
	}

	if (!inv->getSection("gloves"))
	{
		inv->_NV_initialiseNewSection(
			"gloves",
			2, 2,
			ATTACH_GLOVES,
			true, false, true,
			4);
	}
}

// ---------- Hooks ----------
bool (*Character_NV_setupInventorySections_orig)(Character* thisptr, GameSaveState* state) = 0;
void (*Character_NV_loadFromSerialise_orig)(Character* thisptr, GameSaveState* state) = 0;

bool Character_NV_setupInventorySections_Hook(Character* thisptr, GameSaveState* state)
{
	bool result = Character_NV_setupInventorySections_orig(thisptr, state);

	if (thisptr && thisptr->_NV_getInventory())
	{
		ensureExtraInventorySections(thisptr->_NV_getInventory());
	}

	return result;
}

void Character_NV_loadFromSerialise_Hook(Character* thisptr, GameSaveState* state)
{
	if (thisptr && thisptr->_NV_getInventory())
	{
		ensureExtraInventorySections(thisptr->_NV_getInventory());
	}

	Character_NV_loadFromSerialise_orig(thisptr, state);
}

// ---------- Startup ----------
__declspec(dllexport) void startPlugin()
{
	if (KenshiLib::SUCCESS != KenshiLib::AddHook(
		KenshiLib::GetRealAddress(&Character::_NV_setupInventorySections),
		&Character_NV_setupInventorySections_Hook,
		&Character_NV_setupInventorySections_orig))
	{
		ErrorLog("[Extra Inventory Slots] Failure hooking Character::_NV_setupInventorySections.");
	}

	if (KenshiLib::SUCCESS != KenshiLib::AddHook(
		KenshiLib::GetRealAddress(&Character::_NV_loadFromSerialise),
		&Character_NV_loadFromSerialise_Hook,
		&Character_NV_loadFromSerialise_orig))
	{
		ErrorLog("[Extra Inventory Slots] Failure hooking Character::_NV_loadFromSerialise.");
	}
}
