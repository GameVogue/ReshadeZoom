#include <reshade.hpp>
#include <reshade/addon.hpp>
#include <reshade/input.hpp>
#include <unordered_map>

extern "C" __declspec(dllexport) const char* NAME = "ReShade Zoom Plugin";
extern "C" __declspec(dllexport) const char* DESCRIPTION = "A plugin for handling zooming in of the screen to simulate variable zoom scopes.";

static std::unordered_map<reshade::api::effect_runtime *, bool> toggle_states;

// Callback to listen for keyboard input
void on_input(reshade::api::effect_runtime *runtime, const reshade::input::input_event &event)
{
    if (event.type == reshade::input::input_event_type::key_down &&
        event.key_data.vkey == VK_F2) // Use F2 as the toggle key
    {
        // Toggle state
        bool &state = toggle_states[runtime];
        state = !state;

        // Set uniform variable in shader
        reshade::api::effect_uniform_variable toggle_var = runtime->find_uniform_variable({}, "DynamicZoomLevel");
        if (toggle_var != nullptr)
        {
            float value = state ? 2.0f : 1.0f;
            reshade::invoke_addon_event<reshade::addon_event::set_uniform_value>(toggle_var, &value, 1);
        }
    }
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID)
{
    if (ul_reason_for_call == DLL_PROCESS_ATTACH)
    {
        reshade::register_addon(hModule);
        reshade::register_event<reshade::addon_event::input_event>(on_input);
    }
    else if (ul_reason_for_call == DLL_PROCESS_DETACH)
    {
        reshade::unregister_event<reshade::addon_event::input_event>(on_input);
        reshade::unregister_addon(hModule);
    }
    return TRUE;
}

