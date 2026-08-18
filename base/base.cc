#include <SDK/sdhd_sdk.hh>
using namespace UFG;

namespace InGameOnUpdate
{
    SDK::Hook gHook;

    void __fastcall Fn(GameState* game_state, f32 deltaTime)
    {
        gHook(game_state, deltaTime);
    }
}

int __stdcall DllMain(HMODULE module, DWORD reason, void* reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        if (!SDK::IsValidExecutable()) { return 0; }

        InGameOnUpdate::gHook.I_GameStateInGame(SDK::Hook::GameState_OnUpdate, InGameOnUpdate::Fn);
    }

    return 1;
}