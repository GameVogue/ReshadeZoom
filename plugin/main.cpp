#include <reshade.hpp>
#include <reshade_api.hpp>
#include <unordered_map>

// Windows Header Files:
#include <windows.h>
#include <iostream>
#include <string>
#include <tchar.h>
#include <Psapi.h>
#include <utility>
#include <vector>

extern "C" __declspec(dllexport) const char* NAME = "ReShade Zoom Plugin";
extern "C" __declspec(dllexport) const char* DESCRIPTION = "A plugin for handling zooming in of the screen to simulate variable zoom scopes.";

static std::unordered_map<reshade::api::effect_runtime *, bool> toggle_states;


// from ReShade
static const char *keyboard_keys[256] = {
	"", "Left Mouse", "Right Mouse", "Cancel", "Middle Mouse", "X1 Mouse", "X2 Mouse", "", "Backspace", "Tab", "", "", "Clear", "Enter", "", "",
	"Shift", "Control", "Alt", "Pause", "Caps Lock", "", "", "", "", "", "", "Escape", "", "", "", "",
	"Space", "Page Up", "Page Down", "End", "Home", "Left Arrow", "Up Arrow", "Right Arrow", "Down Arrow", "Select", "", "", "Print Screen", "Insert", "Delete", "Help",
	"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "", "", "", "", "", "",
	"", "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O",
	"P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z", "Left Windows", "Right Windows", "Apps", "", "Sleep",
	"Numpad 0", "Numpad 1", "Numpad 2", "Numpad 3", "Numpad 4", "Numpad 5", "Numpad 6", "Numpad 7", "Numpad 8", "Numpad 9", "Numpad *", "Numpad +", "", "Numpad -", "Numpad Decimal", "Numpad /",
	"F1", "F2", "F3", "F4", "F5", "F6", "F7", "F8", "F9", "F10", "F11", "F12", "F13", "F14", "F15", "F16",
	"F17", "F18", "F19", "F20", "F21", "F22", "F23", "F24", "", "", "", "", "", "", "", "",
	"Num Lock", "Scroll Lock", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"Left Shift", "Right Shift", "Left Control", "Right Control", "Left Menu", "Right Menu", "Browser Back", "Browser Forward", "Browser Refresh", "Browser Stop", "Browser Search", "Browser Favorites", "Browser Home", "Volume Mute", "Volume Down", "Volume Up",
	"Next Track", "Previous Track", "Media Stop", "Media Play/Pause", "Mail", "Media Select", "Launch App 1", "Launch App 2", "", "", "OEM ;", "OEM +", "OEM ,", "OEM -", "OEM .", "OEM /",
	"OEM ~", "", "", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "", "", "", "", "", "OEM [", "OEM \\", "OEM ]", "OEM '", "OEM 8",
	"", "", "OEM <", "", "", "", "", "", "", "", "", "", "", "", "", "",
	"", "", "", "", "", "", "Attn", "CrSel", "ExSel", "Erase EOF", "Play", "Zoom", "", "PA1", "OEM Clear", ""
};

bool isKeyPressed(const reshade::api::effect_runtime* runtime, uint8_t _keyCode, bool _altRequired, bool _shiftRequired, bool _ctrlRequired){
	bool toReturn = runtime->is_key_pressed(_keyCode);
	const bool altPressed = runtime->is_key_down(VK_MENU);;
	const bool shiftPressed = runtime->is_key_down(VK_SHIFT);
	const bool ctrlPressed = runtime->is_key_down(VK_CONTROL);

	toReturn &= ((_altRequired && altPressed) || (!_altRequired && !altPressed));
	toReturn &= ((_shiftRequired && shiftPressed) || (!_shiftRequired && !shiftPressed));
	toReturn &= ((_ctrlRequired && ctrlPressed) || (!_ctrlRequired && !ctrlPressed));
	return toReturn;
}

std::string codeToString(uint8_t vkCode){
	return keyboard_keys[vkCode];
}

uint8_t stringTocode(std::string key){
	for (int i=0; i<256; i++) {
		if (keyboard_keys[i] == key) return i;
	}
	return 0;
}

// Callback to listen for keyboard input
static void onReshadePresent(reshade::api::effect_runtime* runtime) {
    if (isKeyPressed(runtime, stringTocode("F2"), false, false, false)) {
        // Variable state
        bool &state = toggle_states[runtime];
        state = !state;

        // Set uniform variable in shader
	const reshade::api::effect_uniform_variable synced_variable = runtime->find_uniform_variable("zoomscope.fx", "ZoomLevelDelta");
        if (synced_variable != 0) {
            float value = state ? 2.0f : 1.0f;
            runtime->set_uniform_value_float(synced_variable, &value, 1);
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        reshade::register_addon(hModule);
	reshade::register_event<reshade::addon_event::reshade_present>(onReshadePresent);
    } else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
	reshade::unregister_event<reshade::addon_event::reshade_present>(onReshadePresent);
        reshade::unregister_addon(hModule);
    }
    return TRUE;
}
