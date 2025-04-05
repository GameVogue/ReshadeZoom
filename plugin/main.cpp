#include <reshade.hpp>
#include <unordered_map>
#include "KeyData.h"

extern "C" __declspec(dllexport) const char* NAME = "ReShade Zoom Plugin";
extern "C" __declspec(dllexport) const char* DESCRIPTION = "A plugin for handling zooming in of the screen to simulate variable zoom scopes.";

static std::unordered_map<reshade::api::effect_runtime *, bool> toggle_states;
static KeyData _keyData;

// Callback to listen for keyboard input
static void onReshadePresent(reshade::api::effect_runtime* runtime) {
    if (_keyData.isKeyPressed(runtime)) {
        // Variable state
        bool &state = toggle_states[runtime];
        state = !state;

        // Set uniform variable in shader
	const reshade::api::effect_uniform_variable synced_variable = runtime->find_uniform_variable({}, "DynamicZoomLevel");
        if (synced_variable != nullptr) {
            float value = state ? 2.0f : 1.0f;
            runtime->set_uniform_value_float(synced_variable, &value, 1);
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        reshade::register_addon(hModule);
	reshade::register_event<reshade::addon_event::reshade_present>(onReshadePresent);
    	_keyData.setKey("F2", false, false, false);
    } else if (ul_reason_for_call == DLL_PROCESS_DETACH) {
	reshade::unregister_event<reshade::addon_event::reshade_present>(onReshadePresent);
        reshade::unregister_addon(hModule);
    }
    return TRUE;
}
