// Model Binding
//
// Demonstrates how to dynamically bind a model to a SimObject.
//
// - InitGameSystems: creates a cube model used by the example.
// - InGameOnUpdate: when F6 is pressed, attaches the cube to:
//     - the player's head when on foot
//     - the vehicle's trunk when the player is in a vehicle

#include <SDK/sdhd_sdk.hh>
using namespace UFG;

#define HOTKEY_CREATE       VK_F6

Illusion::Model* CreateCubeModel(const u32 name_uid, const f32 cube_size)
{
    auto material = Illusion::Factory::NewMaterial(0, name_uid, 8);
    {
        material->mNumParams = 0;

        // iShader, Illusion.Shader, HK_SIMPLE
        material->SetParam(material->mNumParams++, 0x5C19C934, 0x8B5561A1, 0x192E0D6);

        // iTexture, texDiffuse, Illusion.Texture, DEFAULTWHITE
        material->SetParam(material->mNumParams++, 0xC8377453, 0xDCE06689, 0x8B43FABF, 0xB11369EB);

        // iRasterState, Illusion.RasterState, Illusion.RasterState.DoubleSided
        material->SetParam(material->mNumParams++, 0xC0C265E6, 0x3BC715E0, 0x940FB8EC);

        // iAlphaState, Illusion.AlphaState
        material->SetParam(material->mNumParams++, 0xEB98748F, 0x12C800F2, -1);

        // sbDepthBiasSortLayer, Illusion.StateBlock
        material->SetParam(material->mNumParams++, 0xF173D303, 0x4D04C7F2, 0xAF2B2668);

        // sbSpecularLook, Illusion.StateBlock
        material->SetParam(material->mNumParams++, 0xEA270604, 0x4D04C7F2, 0x241AB391);

        // sbTextureAnim, Illusion.StateBlock
        material->SetParam(material->mNumParams++, 0xB2F5D3F2, 0x4D04C7F2, 0xD0B4527C);
    }

    auto vertexBuffer = Illusion::Factory::NewBuffer(0, qStringHash32("VertexBuffer", name_uid), sizeof(Render::vDynamic) * 24);
    {
        vertexBuffer->mBufferType = Illusion::Buffer::TYPE_VERTEX;
        vertexBuffer->mElementByteSize = sizeof(Render::vDynamic);
        vertexBuffer->mNumElements = 24;

        const qVector2 uv0 = { 0.f, 0.f };
        const qVector2 uv1 = { 1.f, 1.f };
        const qVector3 normal = { 0.f, 0.f, 1.f };
        const qVector3 tangent = { 1.f, 0.f, 0.f };

        auto vertex = static_cast<Render::vDynamic*>(vertexBuffer->mData.Get());

        for (int face = 0; 6 > face; ++face)
        {
            qVector3 normal, tangent, bitangent;

            if (0 == face) { normal = { 0.f, 0.f, -1.f }; tangent = { 1.f, 0.f, 0.f }; bitangent = { 0.f, 1.f, 0.f }; } // back
            else if (1 == face) { normal = { 0.f, 0.f,  1.f }; tangent = { 1.f, 0.f, 0.f }; bitangent = { 0.f, 1.f, 0.f }; } // front
            else if (2 == face) { normal = { -1.f, 0.f, 0.f }; tangent = { 0.f, 1.f, 0.f }; bitangent = { 0.f, 0.f, 1.f }; } // left
            else if (3 == face) { normal = { 1.f, 0.f, 0.f }; tangent = { 0.f, 1.f, 0.f }; bitangent = { 0.f, 0.f, 1.f }; } // right
            else if (4 == face) { normal = { 0.f, -1.f, 0.f }; tangent = { 1.f, 0.f, 0.f }; bitangent = { 0.f, 0.f, 1.f }; } // bottom
            else { normal = { 0.f,  1.f, 0.f }; tangent = { 1.f, 0.f, 0.f }; bitangent = { 0.f, 0.f, 1.f }; } // top

            const qVector3 center = { normal.x * cube_size, normal.y * cube_size, normal.z * cube_size };

            for (int corner = 0; 4 > corner; ++corner, ++vertex)
            {
                const f32 su = (corner & 1) ? 1.f : -1.f;
                const f32 sv = (corner & 2) ? 1.f : -1.f;

                vertex->mPosition = {
                    center.x + tangent.x * su * cube_size + bitangent.x * sv * cube_size,
                    center.y + tangent.y * su * cube_size + bitangent.y * sv * cube_size,
                    center.z + tangent.z * su * cube_size + bitangent.z * sv * cube_size
                };
                vertex->mNormal = normal;
                vertex->mTangent = tangent;
                vertex->SetUV(uv0, uv1);

                const u8 r = (vertex->mPosition.x > 0.f) ? 255 : 64;
                const u8 g = (vertex->mPosition.y > 0.f) ? 255 : 64;
                const u8 b = (vertex->mPosition.z > 0.f) ? 255 : 64;

                vertex->SetColor(r, g, b);
            }
        }
    }

    auto indexBuffer = Illusion::Factory::NewBuffer(0, qStringHash32("IndexBuffer", name_uid), sizeof(u16) * 36);
    {
        indexBuffer->mBufferType = Illusion::Buffer::TYPE_INDEX;
        indexBuffer->mElementByteSize = sizeof(u16);
        indexBuffer->mNumElements = 36;

        auto index = static_cast<u16*>(indexBuffer->mData.Get());

        for (u16 face = 0; 6 > face; ++face)
        {
            const u16 base = face * 4;

            *index++ = base + 0;
            *index++ = base + 1;
            *index++ = base + 2;

            *index++ = base + 2;
            *index++ = base + 1;
            *index++ = base + 3;
        }
    }

    auto model = Illusion::Factory::NewModel(0, name_uid, 1);
    {
        model->mAABBMin[0] = model->mAABBMin[1] = model->mAABBMin[2] = -cube_size * 10.f;
        model->mAABBMax[0] = model->mAABBMax[1] = model->mAABBMax[2] = cube_size * 10.f;

        auto mesh = model->GetMesh(0);
        {
            mesh->mMaterialHandle.mNameUID = material->mNode.mUID;
            mesh->mVertexDeclHandle.Init(RTypeUID_VertexDecl, 0x64A43DAE);

            mesh->mIndexBufferHandle.mNameUID = indexBuffer->mNode.mUID;
            mesh->mVertexBufferHandles[0].mNameUID = vertexBuffer->mNode.mUID;

            mesh->mPrimType = Illusion::Primitive::TRIANGLELIST;
            mesh->mNumPrims = indexBuffer->mNumElements / 3;
        }
    }

    auto resourceWarehouse = qResourceWarehouse::Instance();

    resourceWarehouse->Add(material);
    resourceWarehouse->Add(vertexBuffer);
    resourceWarehouse->Add(indexBuffer);
    resourceWarehouse->Add(model);

    return model;
}

Illusion::Model* gModel = 0;

namespace InitGameSystems
{
    SDK::Hook gHook;

    void __fastcall Fn()
    {
        gHook();

        gModel = CreateCubeModel(qStringHash32("TestCube"), 0.25f);
    }
}

namespace InGameOnUpdate
{
    SDK::Hook gHook;

    void __fastcall Fn(GameState* game_state, f32 deltaTime)
    {
        gHook(game_state, deltaTime);

        static bool sWasPressed;

        const bool pressed = GetAsyncKeyState(HOTKEY_CREATE);

        if (pressed && !sWasPressed)
        {
            auto lp = GetLocalPlayer();
            SimObjectCVBase* obj = lp;
            const char* bone_name = "Bip01 Head";

            if (auto occupant = lp->GetCharacterOccupant()) 
            {
                if (auto vehicle = occupant->GetCurrentVehicle()) 
                {
                    obj = vehicle;
                    bone_name = "M_Trunk";
                }
            }

            auto compositeDrawable = obj->GetCompositeDrawable();

            auto modelType = compositeDrawable->mModelType[0];

            auto rigResource = static_cast<RigResource*>(modelType->mRigHandle.mData);
            int boneID = rigResource->mUFGSkeleton->GetBoneID(qStringHashUpper32(bone_name));

            Illusion::ModelProxy modelProxy;
            modelProxy.Init(gModel->mNode.mUID);

            modelType->AddRigidModelBinding(modelProxy, boneID, 69);
        }

        sWasPressed = pressed;
    }
}

int __stdcall DllMain(HMODULE module, DWORD reason, void* reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        if (!SDK::IsValidExecutable()) { return 0; }

        InitGameSystems::gHook.I_InitGameSystems(InitGameSystems::Fn);
        InGameOnUpdate::gHook.I_GameStateInGame(SDK::Hook::GameState_OnUpdate, InGameOnUpdate::Fn);
    }

    return 1;
}