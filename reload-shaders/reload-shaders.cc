#include <SDK/sdhd_sdk.hh>
using namespace UFG;

#define HOTKEY_VK       'P'

bool __fastcall LoadShaderBinary(qChunk* chunk)
{
    auto* new_bin = static_cast<Illusion::ShaderBinary*>(chunk->GetData());

    auto* resoruceWarehouse = qResourceWarehouse::Instance();
    auto* inventory = resoruceWarehouse->GetInventory(RTypeUID_ShaderBinary);
    auto* bin = static_cast<Illusion::ShaderBinary*>(inventory->Get(new_bin->mNode.mUID));
    if (!bin)
    {
        auto* resource = static_cast<qResourceData*>(malloc(chunk->mDataSize));
        memcpy(resource, new_bin, chunk->mDataSize);
        inventory->Add(resource);
        return 0;
    }

    if (bin->mShaderStageType != new_bin->mShaderStageType) {
        return 0;
    }

    auto name = qString(bin->mShaderName).GetFilenameWithoutExtension();
    auto* shader = static_cast<Illusion::Shader*>(resoruceWarehouse->DebugGet(RTypeUID_Shader, qStringHash32(name)));
    if (!shader) {
        return 0;
    }

    auto* plat = shader->GetPlat();
    auto* stageShader = plat->mShaderStage[new_bin->mShaderStageType];

    if (stageShader->LoadShader(new_bin->mData.Get(), new_bin->mDataByteSize, static_cast<Illusion::Shader::StageType>(new_bin->mShaderStageType)))
    {
        auto* shaderSystemPlat = Illusion::ShaderSystemPlat::Instance();
        shaderSystemPlat->AddStageShaderResources(shader, stageShader);
        shaderSystemPlat->ValidShaderStageInputs(shader);
    }

    return 0;
}

namespace InGameOnUpdate
{
    SDK::Hook gHook;

    void __fastcall Fn(GameState* game_state, f32 deltaTime)
    {
        gHook(game_state, deltaTime);

        static bool sWasPressed;

        const bool pressed = GetAsyncKeyState(HOTKEY_VK);

        if (pressed && !sWasPressed)
        {
            i64 size = 0;
            if (void* buf = StreamFileWrapper::ReadEntireFile("Data\\Shaders\\Shaders.temp.bin", &size))
            {
                StreamResourceLoader::Load(buf, static_cast<u32>(size), 0, StreamerMetrics::DATA_UNKNOWN, LoadShaderBinary);
                qFree(buf);
            }
        }

        sWasPressed = pressed;
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