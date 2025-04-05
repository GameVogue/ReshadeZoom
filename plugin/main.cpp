#include <reshade.hpp>
#include <reshade_api.hpp>
#include <algorithm>

extern "C" __declspec(dllexport) const char* NAME = "ReShade Zoom Plugin";
extern "C" __declspec(dllexport) const char* DESCRIPTION = "A plugin for handling zooming in of the screen to simulate variable zoom scopes.";

// Callback to listen for keyboard input
static void onReshadePresent(reshade::api::effect_runtime* runtime) {
    const reshade::api::effect_uniform_variable zoom_var = runtime->find_uniform_variable("zoomscope.fx", "DynamicZoomLevel");
    const reshade::api::effect_uniform_variable wheel_var = runtime->find_uniform_variable("zoomscope.fx", "MouseWheelDelta");
    const reshade::api::effect_uniform_variable scale_var = runtime->find_uniform_variable("zoomscope.fx", "ZoomLevelDelta");
    const reshade::api::effect_uniform_variable enable_var = runtime->find_uniform_variable("zoomscope.fx", "EnableMagnifier");
    if (zoom_var == 0 || wheel_var == 0 || scale_var == 0 || enable_var == 0) return;
    bool enabled = false;
    runtime->get_uniform_value_bool(enable_var, &enabled, 1);
    if (!enabled) {
        float DynamicZoomLevel = 1.0f;
        runtime->set_uniform_value_float(zoom_var, &DynamicZoomLevel, 1);
    } else {
        float DynamicZoomLevel = 2.0f;
        runtime->set_uniform_value_float(zoom_var, &DynamicZoomLevel, 1);
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
