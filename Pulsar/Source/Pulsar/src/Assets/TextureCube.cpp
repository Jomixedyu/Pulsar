#include "Assets/TextureCube.h"

#include "Application.h"

#include <ranges>

namespace pulsar
{
    static void GeneratePlaneX(array_list<StaticMeshVertex>& arr, float _x, float _y, float _z, float dir)
    {
        float x = _x * dir;
        float y = _y;
        float z = _z * dir;
        float n = dir;
        arr.push_back({{x, y, -z}, {n, 0, 0}});
        arr.push_back({{x, -y, -z}, {n, 0, 0}});
        arr.push_back({{x, y, z}, {n, 0, 0}});
        arr.push_back({{x, y, z}, {n, 0, 0}});
        arr.push_back({{x, -y, -z}, {n, 0, 0}});
        arr.push_back({{x, -y, z}, {n, 0, 0}});
    }
    static void GeneratePlaneY(array_list<StaticMeshVertex>& arr, float _x, float _y, float _z, float dir)
    {
        float x = _x;
        float y = _y * dir;
        float z = _z * dir;
        float n = dir;
        arr.push_back({{-x, y, z}, {0, n, 0}});
        arr.push_back({{-x, y, -z}, {0, n, 0}});
        arr.push_back({{x, y, z}, {0, n, 0}});
        arr.push_back({{x, y, z}, {0, n, 0}});
        arr.push_back({{-x, y, -z}, {0, n, 0}});
        arr.push_back({{x, y, -z}, {0, n, 0}});
    }
    static void GeneratePlaneZ(array_list<StaticMeshVertex>& arr, float _x, float _y, float _z, float dir)
    {
        float x = _x * dir;
        float y = _y;
        float z = _z * dir;
        float n = dir;
        arr.push_back({{x, y, z}, {0, 0, n}});
        arr.push_back({{x, -y, z}, {0, 0, n}});
        arr.push_back({{-x, y, z}, {0, 0, n}});
        arr.push_back({{-x, y, z}, {0, 0, n}});
        arr.push_back({{x, -y, z}, {0, 0, n}});
        arr.push_back({{-x, -y, z}, {0, 0, n}});
    }

    static array_list<StaticMeshVertex> GenerateCube(float x, float y, float z)
    {
        array_list<StaticMeshVertex> vert;
        vert.reserve(36);
        constexpr float extent = 1;
        GeneratePlaneX(vert, extent, extent, extent, 1);
        GeneratePlaneX(vert, extent, extent, extent, -1);
        GeneratePlaneY(vert, extent, extent, extent, 1);
        GeneratePlaneY(vert, extent, extent, extent, -1);
        GeneratePlaneZ(vert, extent, extent, extent, 1);
        GeneratePlaneZ(vert, extent, extent, extent, -1);
        return vert;
    }

    bool TextureCube::CreateGPUResource()
    {
        if (m_isCreatedGPUResource)
        {
            return true;
        }

        auto gfxapp = Application::GetGfxApp();

        m_cube = gfxapp->CreateTextureCube(m_width);

        auto texCube = m_cube;

        auto renderFunction = [texCube](gfx::GFXCommandBuffer* cmd) {
            // load material
            RCPtr<Material> mat;
            auto gfxapp = cmd->GetApplication();

            array_list<gfx::GFXTexture2DView_sp> faceRts;
            faceRts.resize(6);
            for (int i = 0; i < 6; ++i)
            {
                faceRts[i] = texCube->Get2DView(i);
            }
            auto facePointers = faceRts
                | std::views::transform([](gfx::GFXTexture2DView_sp& x){ return x.get(); })
                | std::ranges::to<array_list<gfx::GFXTexture2DView*>>();

            gfx::GFXRenderPassLayout_sp renderPass = gfxapp->CreateRenderPassLayout(facePointers);

            gfx::GFXFrameBufferObject_sp frameBuffers[6]{};
            for (auto& fbo : frameBuffers)
            {
                fbo = gfxapp->CreateFrameBufferObject(faceRts, renderPass);
            }

            // render
            for (int i = 0; i < 6; ++i)
            {
                auto& fbo = frameBuffers[i];
                cmd->SetFrameBuffer(fbo.get());
                cmd->CmdBeginFrameBuffer();



                cmd->CmdEndFrameBuffer();
                cmd->SetFrameBuffer(nullptr);
            }

            for (auto& fbo : frameBuffers)
            {
                cmd->SetFrameBuffer(fbo.get());
                cmd->CmdBeginFrameBuffer();





                cmd->CmdEndFrameBuffer();
                cmd->SetFrameBuffer(nullptr);
            }


        };
        // gfxapp->GetRenderer()->WaitExecuteRender(renderFunction);


        // wait render hdr to cube

        m_isCreatedGPUResource = true;

        return true;
    }
    void TextureCube::DestroyGPUResource()
    {
        if (!m_isCreatedGPUResource)
        {
            return;
        }
        m_cube.reset();
    }
} // namespace pulsar
