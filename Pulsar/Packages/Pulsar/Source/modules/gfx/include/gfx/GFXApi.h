#pragma once

namespace gfx
{
    enum class GFXApi
    {
        Unknown,
        D3D12,
        Vulkan,
    };

    inline const char* to_string(GFXApi api)
    {
        switch (api)
        {
        case gfx::GFXApi::Unknown: return "NONE";
        case gfx::GFXApi::D3D12: return "D3D12";
        case gfx::GFXApi::Vulkan: return "Vulkan";
        }
        return nullptr;
    }
}