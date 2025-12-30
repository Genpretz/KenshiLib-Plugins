// FactionHealing.cpp - RE_Kenshi Plugin for Automatic Faction-Based Healing
// Author: Matrix Agent - "Well, to be honest i don't want to improve, compile and test it, i don't have VS here and i'm too lazy for that. I'm also not familiar with Kenshi modding. So anyone can take the credit if that idea is possible."
// Edited by: Genpretz - "I needed to adjust the code in order to make it compatible with the v100 platform toolset."
// Description: Allows player characters to automatically heal unconscious NPCs from selected factions

#include <windows.h>
#include <string>
#include <vector>
#include <fstream>
#include <algorithm>

#include <kenshi/Character.h>
#include <kenshi/MedicalSystem.h>
#include <kenshi/Faction.h>
#include <kenshi/Globals.h>
#include <core/Functions.h>

// ============================================================================
// LIST OF FACTIONS ENABLED FOR HEALING
// Edit this file to add/remove factions
// ============================================================================

const char* ENABLED_FACTIONS[] =
{
	 //"Anti-Slavers",
	 //"Band of Bones",
	 //"Bar Thugs",
	 //"Berserkers",
	 //"Black Desert Ninjas",
	 //"Black Dog",
	 //"Black Dragon Ninjas",
	 //"Blackshifters",
	 //"Bloodraiders",
	 //"Bounty Hunters",
	 //"Cannibal Hunters",
	 //"Cannibals",
	 //"Crab Raiders",
	 //"Deadcat",
	 //"Drifters",
	 //"Dust Bandits",
	 //"Empire Peasants",
	 //"Fishmen",
	 //"Flotsam Ninjas",
	 //"Fogmen",
	 //"Free Traders",
	 //"Grass Pirates",
	 //"Grayflayers",
	 //"Holy Nation Outlaws",
	 //"Hounds",
	 //"Imperial Lords",
	 //"Kral's Chosen",
	 //"Machinists",
	 //"Manhunters",
	 //"Mercenary Guild",
	 //"Mongrel",
	 //"Nomads",
	 //"Outlaw",
	 //"Police",
	 //"Preacher Cult",
	 //"Reavers",
	 //"Rebel Farmers",
	 //"Rebel Swordsmen",
	 //"Rebirth Slaves",
	 //"Red Sabres",
	 //"Sand Ninjas",
	 //"Scavengers",
	 //"Second Empire",
	 //"Second Empire Exile",
	 //"Shek Kingdom",
	 //"Shinobi Thieves",
	 //"Shrieking Bandits",
	 //"Skeleton Bandits",
	 //"Skeleton Legion",
	 //"Skeletons",
	 //"Skin Bandits",
	 //"Slave Hunters",
	 //"Slave Traders",
	 //"Slaves",
	 //"Southern Hive",
	 //"Starving Bandits",
	 //"Stone Rats",
	 //"Swamp Ninjas",
	 //"Swampers",
	 //"Tech Hunters",
	 //"The Gorrillo Bandits",
	 "The Holy Nation",
	 //"Thrall Masters",
	 //"Trade Ninjas",
	 //"Traders Guild",
	 //"Twinblades",
	 "United Cities",
	 //"United Heroes League",
	 //"Vagrants",
	 //"Western Hive",
	 //"Yabuta Outlaws",
	 //"Fishermen"
};

const std::vector<std::string>& GetEnabledFactions()
{
	static std::vector<std::string> factions;
	
	if (factions.empty())
	{
		const size_t count =
			sizeof(ENABLED_FACTIONS) / sizeof(ENABLED_FACTIONS[0]);

		for (size_t i = 0; i < count; ++i)
			factions.push_back(ENABLED_FACTIONS[i]);
	}

	return factions;
}




// ============================================================================
// HOOK IMPLEMENTATION
// ============================================================================

// Pointer to the original function
bool (*shouldIHelpThisGuy_orig)(Character* thisptr, Character* who);

// Checks if the faction is in the enabled list
bool isFactionEnabled(const std::string& factionName)
{
	const std::vector<std::string>& enabled = GetEnabledFactions();


	for (std::vector<std::string>::const_iterator it = enabled.begin();
		it != enabled.end();
		++it)
	{
		if (*it == factionName)
			return true;
	}

	return false;
}

// Hook for shouldIHelpThisGuy function
// This function is called when a character decides whether to help another
bool shouldIHelpThisGuy_hook(Character* thisptr, Character* who)
{
	// Null pointer validation
	if (who == nullptr)
		return shouldIHelpThisGuy_orig(thisptr, who);

	// Get the faction of the character needing help
	Faction* targetFaction = who->getFaction();
	if (targetFaction == nullptr)
		return shouldIHelpThisGuy_orig(thisptr, who);

	// Get the faction name
	const std::string& factionName = targetFaction->getName();

	// If the faction is enabled, force healing
	if (isFactionEnabled(factionName))
	{
		// Check if the character actually needs medical help
		// (is unconscious or severely wounded)
		if (who->_NV_isUnconcious())
		{
			return true; // Force the character to help
		}
	}

	// Otherwise, use the game's default behavior
	return shouldIHelpThisGuy_orig(thisptr, who);
}

// ============================================================================
// PLUGIN ENTRY POINT
// ============================================================================

// Exported function that RE_Kenshi calls when loading the plugin
__declspec(dllexport) void startPlugin()
{
	// Register the hook on shouldIHelpThisGuy function
	KenshiLib::AddHook(
		KenshiLib::GetRealAddress(&Character::shouldIHelpThisGuy),
		shouldIHelpThisGuy_hook,
		&shouldIHelpThisGuy_orig
	);
}
