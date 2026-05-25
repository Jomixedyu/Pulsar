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
        m_isCreatedGPUResource = true;
        if (!m_proxy)
            m_proxy = mksptr(new RenderProxyTextureCube(this));
        m_proxy->InitRHI();
        return true;
    }

    void TextureCube::DestroyGPUResource()
    {
        if (!m_isCreatedGPUResource)
        {
            return;
        }
        m_isCreatedGPUResource = false;
        if (m_proxy)
            m_proxy->ReleaseRHI();
        m_proxy.reset();
    }

    std::shared_ptr<gfx::GFXTexture> TextureCube::GetGFXTexture() const
    {
        if (m_proxy)
            return m_proxy->GetGFXTexture();
        return nullptr;
    }

} // namespace pulsar
