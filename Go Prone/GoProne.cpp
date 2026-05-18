#include <Debug.h>

#include <core/Functions.h>
#include <kenshi/Globals.h>
#include <kenshi/GameWorld.h>
#include <kenshi/PlayerInterface.h>
#include <kenshi/InputHandler.h>
#include <kenshi/Character.h>
//#include <kenshi/gui/DataPanelGUI.h>
//#include <kenshi/gui/DataPanelLine.h>

//#include <boost/locale.hpp>

// Global references declared in KenshiLib's Globals.h
// GameWorld* ou;
// InputHandler* key;

static const DWORD DOUBLE_TAP_WINDOW_MS = 250;

static void ToggleCharacterProneState(hand* handle)
{
    if (!ou || !ou->player || !handle) return;

    Character* c = handle->getCharacter();
    if (!c) return;

    switch (c->_NV_getProneState())
    {
    case PS_NORMAL:
        c->setStealthMode(true);
        c->_NV_setProneState(PS_STAYING_LOW);
        break;

    case PS_STAYING_LOW:
        c->_NV_setProneState(PS_NORMAL);
        c->setStealthMode(false);
        break;

    case PS_PLAYING_DEAD:
    case PS_CRIPPLED:
    case PS_KO:
        break;

    default:
        break;
    }
}

static void CheckSneakDoubleTap()
{
    if (!key) return;

    auto* cmd = key->getCommand("toggle_sneak");
    if (!cmd) return;

    if (key->events.find(cmd) == key->events.end())
        return;

    static DWORD lastTapTime = 0;
    const DWORD now = GetTickCount();
    const DWORD delta = now - lastTapTime;
    lastTapTime = now;

    if (delta > 0 && delta <= DOUBLE_TAP_WINDOW_MS)
    {
        if (!ou || !ou->player) return;

        ogre_unordered_set<hand>::type chars = ou->player->selectedCharacters;

        for (ogre_unordered_set<hand>::type::iterator it = chars.begin();
            it != chars.end();
            ++it)
        {
            hand h = *it;
            ToggleCharacterProneState(&h);
        }
    }
}

// static void CheckToggleProne()
// {
//     if (!key) return;
//
//     auto* cmd = key->getCommand("toggle_prone");
//     if (!cmd) return;
//
//     if (key->events.find(cmd) == key->events.end())
//         return;
//
//     if (!ou || !ou->player) return;
//
//     ogre_unordered_set<hand>::type chars = ou->player->selectedCharacters;
//
//     for (ogre_unordered_set<hand>::type::iterator it = chars.begin();
//         it != chars.end();
//         ++it)
//     {
// 		hand h = *it;
//         ToggleCharacterProneState(&h);
//     }
// }

//void (*DatapanelGUI_addCustomLine_orig)(DatapanelGUI* thisptr, DataPanelLine* line);
//void DatapanelGUI_addCustomLine_hook(DatapanelGUI* thisptr, DataPanelLine* line)
//{
//    DatapanelGUI_addCustomLine_orig(thisptr, line);
//    if (line->s1 == "Toggle stealth mode")
//        thisptr->addCustomLine(new DataPanelLine_KeyConfig("toggle_prone", boost::locale::gettext("Toggle Prone"), 25));
//}
//
//void (*InputHandler_loadConfig_orig)(InputHandler* thisptr);
//void InputHandler_loadConfig_hook(InputHandler* thisptr)
//{
//    thisptr->addCommand("toggle_prone", false, OIS::KeyCode::KC_UNASSIGNED, OIS::KeyCode::KC_UNASSIGNED, InputHandler::NONE_MASK, InputHandler::GLOBAL);
//    InputHandler_loadConfig_orig(thisptr);
//}

static void (*InputHandler_keyDownEvent_orig)(InputHandler*, OIS::KeyCode) = nullptr;
static void InputHandler_keyDownEvent_hook(InputHandler* thisptr, OIS::KeyCode keyCode)
{
    if (InputHandler_keyDownEvent_orig)
        InputHandler_keyDownEvent_orig(thisptr, keyCode);

    if (ou && ou->player && ou->player->selectedCharacter)
    {
        CheckSneakDoubleTap();
    }
}

__declspec(dllexport) void __cdecl startPlugin()
{
    if (KenshiLib::SUCCESS != KenshiLib::AddHook(KenshiLib::GetRealAddress(&InputHandler::keyDownEvent), InputHandler_keyDownEvent_hook, &InputHandler_keyDownEvent_orig))
    {
        ErrorLog("Could not hook InputHandler::keyDownEvent");
    }
    /*if (KenshiLib::SUCCESS != KenshiLib::AddHook(KenshiLib::GetRealAddress(&InputHandler::loadConfig), InputHandler_loadConfig_hook, &InputHandler_loadConfig_orig))
    {
        ErrorLog("Could not hook InputHandler::loadConfig");
    }
    if (KenshiLib::SUCCESS != KenshiLib::AddHook(KenshiLib::GetRealAddress(&DatapanelGUI::addCustomLine), DatapanelGUI_addCustomLine_hook, &DatapanelGUI_addCustomLine_orig))
    {
        ErrorLog("Could not hook DatapanelGUI::addCustomLine");
    }*/

    //InputHandler_loadConfig_orig(key);
}