#include <Debug.h>

#include <core/Functions.h>

#include <kenshi/Character.h>
#include <kenshi/GameWorld.h>
#include <kenshi/Globals.h>
#include <kenshi/InputHandler.h>
#include <kenshi/PlayerInterface.h>

// Global references declared in KenshiLib's Globals.h
// GameWorld* ou;
// InputHandler* key;

static const DWORD DOUBLE_TAP_WINDOW_MS = 250;

static void ToggleCharacterProneState()
{
    if (!ou || !ou->player)
    {
        return;
    }

    Character* c = ou->player->selectedCharacter.getCharacter();
    if (!c)
    {
        ErrorLog("[Go Prone Plugin] No selected character found.\n");
        return;
    }

    switch (c->_NV_getProneState())
    {
    case PS_NORMAL:
    {
        DebugLog("[Go Prone Plugin] PS_NORMAL -> PS_STAYING_LOW\n");
        c->setStealthMode(true);
        c->_NV_setProneState(PS_STAYING_LOW);
        break;
    }
    case PS_STAYING_LOW:
    {
        DebugLog("[Go Prone Plugin] PS_STAYING_LOW -> PS_NORMAL\n");
        c->_NV_setProneState(PS_NORMAL);
        c->setStealthMode(false);
        break;
    }
    case PS_PLAYING_DEAD:
    case PS_CRIPPLED:
    case PS_KO:
    {
        DebugLog("[Go Prone Plugin] Leaving proneState as is.\n");
        break;
    }
    default:
    {
        break;
    }
    }
}

static void CheckSneakDoubleTap()
{
    if (!key)
    {
        return;
    }

    InputHandler::Command* cmd = key->getCommand("toggle_sneak");
    if (!cmd)
    {
        return;
    }

    if (key->events.find(cmd) == key->events.end())
    {
        return;
    }

    static DWORD lastTapTime = 0;

    const DWORD now = GetTickCount();
    const DWORD delta = now - lastTapTime;
    lastTapTime = now;

    if (delta > 0 && delta <= DOUBLE_TAP_WINDOW_MS)
    {
        ToggleCharacterProneState();
    }
}

static void (*InputHandler_keyDownEvent_orig)(InputHandler*, OIS::KeyCode) = nullptr;

static void InputHandler_keyDownEvent_hook(InputHandler* thisptr, OIS::KeyCode keyCode)
{
    if (InputHandler_keyDownEvent_orig)
    {
        InputHandler_keyDownEvent_orig(thisptr, keyCode);
    }

    if (ou && ou->player && ou->player->selectedCharacter)
    {
        CheckSneakDoubleTap();
    }
}

__declspec(dllexport) void __cdecl startPlugin()
{
    if (KenshiLib::SUCCESS != KenshiLib::AddHook(
        KenshiLib::GetRealAddress(&InputHandler::keyDownEvent),
        &InputHandler_keyDownEvent_hook,
        &InputHandler_keyDownEvent_orig))
    {
        ErrorLog("[Go Prone Plugin] Could not hook InputHandler::keyDownEvent\n");
    }
}