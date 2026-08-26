#include <Debug.h>

#include <core/Functions.h>
#include <kenshi/Globals.h>
#include <kenshi/GameWorld.h>
#include <kenshi/PlayerInterface.h>
#include <kenshi/InputHandler.h>
#include <kenshi/Character.h>
#include <kenshi/gui/DataPanelGUI.h>
#include <kenshi/gui/DataPanelLine.h>
#include <kenshi/gui/ForgottenGUI.h>
#include <kenshi/gui/MainBarGUI.h>
#include <kenshi/gui/OrdersPanel.h>
#include <mygui/MyGUI_Button.h>

#include <boost/locale.hpp>

// Global references declared in KenshiLib's Globals.h
// GameWorld* ou;
// InputHandler* key;
// ForgottenGUI* gui;

static void SetCharacterProneState(const hand& handle, bool goProne)
{
    Character* c = handle.getCharacter();
    if (!c) return;

    ProneState currentState = c->getProneState();
    if (currentState == PS_PLAYING_DEAD || currentState == PS_CRIPPLED || currentState == PS_KO) return;

    if (goProne)
    {
        c->setStealthMode(true);
        c->setProneState(PS_STAYING_LOW);
    }
    else
    {
        c->setProneState(PS_NORMAL);
        c->setStealthMode(true); // Keep stealth mode on when standing up, otherwise the character will stand up and become visible to enemies.
    }
}

static void CheckToggleProne(PlayerInterface* player, InputHandler& keyHandler)
{
    if (!player || !keyHandler.events.size()) return;

    InputHandler::Command* cmd = keyHandler.getCommand("toggle_prone");
    if (!cmd) return;

    if (keyHandler.events.find(cmd) == keyHandler.events.end()) return;

    // If any selected character is currently laying low, stand them up; otherwise, go prone
    bool targetProne = !player->selectedCharactersLayingLow();

    const ogre_unordered_set<hand>::type& chars = player->selectedCharacters;
    for (ogre_unordered_set<hand>::type::const_iterator it = chars.begin(); it != chars.end(); ++it)
    {
        SetCharacterProneState(*it, targetProne);
    }

    if (gui && gui->mainbar && gui->mainbar->ordersDataPanel && gui->mainbar->ordersDataPanel->stealthCheckBox)
    {
        Character* focused = player->selectedCharacter.getCharacter();
        if (!focused && !chars.empty())
        {
            focused = chars.begin()->getCharacter();
        }

        if (focused)
        {
            gui->mainbar->ordersDataPanel->stealthCheckBox->setStateSelected(focused->isStealthMode());
        }
        else
        {
            gui->mainbar->ordersDataPanel->stealthCheckBox->setStateSelected(targetProne);
        }
    }
}

void (*DatapanelGUI_addCustomLine_orig)(DatapanelGUI* thisptr, DataPanelLine* line);
void DatapanelGUI_addCustomLine_hook(DatapanelGUI* thisptr, DataPanelLine* line)
{
    DatapanelGUI_addCustomLine_orig(thisptr, line);
    if (line->s1 == "Toggle stealth mode" || 
        (line->classType == DataPanelLine::DPL_CUSTOM && static_cast<DataPanelLine_KeyConfig*>(line)->command == "toggle_sneak"))
    {
        thisptr->addCustomLine(new DataPanelLine_KeyConfig("toggle_prone", boost::locale::gettext("Toggle prone"), 25));
    }
}

void (*InputHandler_loadConfig_orig)(InputHandler* thisptr);
void InputHandler_loadConfig_hook(InputHandler* thisptr)
{
    thisptr->addCommand("toggle_prone", 0, OIS::KC_UNASSIGNED, OIS::KC_UNASSIGNED, InputHandler::NONE_MASK, InputHandler::GLOBAL);
    InputHandler_loadConfig_orig(thisptr);
}

void (*PlayerInterface_playerControl_orig)(PlayerInterface* thisptr, InputHandler& key);
void PlayerInterface_playerControl_hook(PlayerInterface* thisptr, InputHandler& key)
{
    PlayerInterface_playerControl_orig(thisptr, key);
    CheckToggleProne(thisptr, key);
}

__declspec(dllexport) void startPlugin()
{
    if (KenshiLib::SUCCESS != KenshiLib::AddHook(KenshiLib::GetRealAddress(&PlayerInterface::playerControl), PlayerInterface_playerControl_hook, &PlayerInterface_playerControl_orig))
    {
        ErrorLog("Could not hook PlayerInterface::playerControl");
    }
    if (KenshiLib::SUCCESS != KenshiLib::AddHook(KenshiLib::GetRealAddress(&InputHandler::loadConfig), InputHandler_loadConfig_hook, &InputHandler_loadConfig_orig))
    {
        ErrorLog("Could not hook InputHandler::loadConfig");
    }
    if (KenshiLib::SUCCESS != KenshiLib::AddHook(KenshiLib::GetRealAddress(&DatapanelGUI::addCustomLine), DatapanelGUI_addCustomLine_hook, &DatapanelGUI_addCustomLine_orig))
    {
        ErrorLog("Could not hook DatapanelGUI::addCustomLine");
    }

    if (key)
    {
        InputHandler_loadConfig_hook(key);
    }
}