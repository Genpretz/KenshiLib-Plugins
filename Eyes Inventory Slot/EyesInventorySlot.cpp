#include <Debug.h>
#include <core/Functions.h>
#include <kenshi/Character.h>
#include <kenshi/Inventory.h>

bool (*Character_setupInventorySections_orig)(Character* thisptr, GameSaveState* state) = nullptr;
bool Character_setupInventorySections_Hook(Character* thisptr, GameSaveState* state)
{
	// Call original setup first  
	bool result = Character_setupInventorySections_orig(thisptr, state);

	// Then add your custom section once  
	thisptr->getInventory()->_NV_initialiseNewSection("eyes", 3, 1, ATTACH_HAT, true, false, true, 1);

	return result;
}

__declspec(dllexport) void startPlugin()
{
	if (KenshiLib::SUCCESS == KenshiLib::AddHook(
		KenshiLib::GetRealAddress(&Character::_NV_setupInventorySections),
		&Character_setupInventorySections_Hook,
		&Character_setupInventorySections_orig))
	{
		DebugLog("[Eyes Inventory Slot Plugin] Character::_NV_setupInventorySections Hooked successfully.\n");
	}
	else
	{
		ErrorLog("[Eyes Inventory Slot Plugin] failed to hook Character::_NV_setupInventorySections.\n");
	}
};

