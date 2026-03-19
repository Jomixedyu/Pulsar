#pragma once
#include "GFXInclude.h"
#include "TextureClasses.h"

namespace gfx
{
    struct GFXRenderTargetDesc
    {
        array_list<GFXTextureFormat> ColorFormats;
        GFXTextureFormat DepthStencilFormat{};
        bool HasDepthStencil = false;

        uint64_t GetHashCode() const
        {
            constexpr uint64_t HashS0 = 2166136261;
            constexpr uint64_t HashS1 = 16777619;
            uint64_t hash = HashS0;
            for (auto fmt : ColorFormats)
            {
                hash = hash * HashS1 ^ static_cast<uint64_t>(fmt);
            }
            hash = hash * HashS1 ^ static_cast<uint64_t>(HasDepthStencil);
            if (HasDepthStencil)
            {
                hash = hash * HashS1 ^ static_cast<uint64_t>(DepthStencilFormat);
            }
            return hash;
        }
    };
}