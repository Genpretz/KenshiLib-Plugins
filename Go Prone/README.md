## Overview

This plugin extends **Kenshi** by adding a **double-tap sneak key mechanic** that toggles the selected character between standing and a low/prone-style stealth stance.

It hooks into the game’s input system using KenshiLib and listens for rapid consecutive presses of the `toggle_sneak` command. If the key is pressed twice within a short time window, the plugin forces a prone-state transition.

---

## How It Works

### 1. Input Hooking

On plugin startup (`startPlugin()`), the plugin:

- Hooks `InputHandler::keyDownEvent`
- Preserves the original function pointer
- Calls the original handler before executing custom logic

This ensures native game input remains fully functional.

---

### 2. Double-Tap Detection

The plugin:

- Retrieves the `toggle_sneak` command
- Confirms the command exists in the input event map
- Tracks the time between key presses using `GetTickCount()`
- Compares the delta against a 250ms threshold

If two presses occur within that window, the plugin toggles the character’s prone state.

The `toggle_sneak` command can be set in the game's controls/keybindings menu or via the Controls.cfg file found in the installation directory. 
The `toggle_sneak` command is bound to the "NUM4" key by default. NUM4 referring to the num keypad 4 key.

I'd like to make the keybind configurable seperately from the `toggle_sneak` command in the future, but for now it is hardcoded to whichever key is associated with the `toggle_sneak` command.

---

### 3. Prone State Transitions

When triggered, the selected character transitions as follows:

| Current State      | New State        | Additional Behavior   |
|-------------------|------------------|------------------------|
| `PS_NORMAL`       | `PS_STAYING_LOW` | Enables stealth mode   |
| `PS_STAYING_LOW`  | `PS_NORMAL`      | Disables stealth mode  |
| `PS_PLAYING_DEAD` | No change        | State preserved        |
| `PS_CRIPPLED`     | No change        | State preserved        |
| `PS_KO`           | No change        | State preserved        |

- Disabling stealth mode when exiting prone is to ensure the character isn't permanently stuck in stealth mode.
- Restricting the states that can be toggled is meant to prevent unintended behavior during incapacitation.
