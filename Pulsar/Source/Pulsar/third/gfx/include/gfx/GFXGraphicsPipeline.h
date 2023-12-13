#pragma once
#include "GFXShaderPass.h"

namespace gfx
{

    struct GFXGraphicsPipelineState
    {
        GFXPrimitiveTopology Topology{};
        float LineWidth{1.0};
        bool IsReverseCulling{false};

        size_t GetHashCode() const
        {
            constexpr size_t prime = 16777619;
            size_t hash = 2166136261;
            hash = (hash ^ std::hash<GFXPrimitiveTopology>()(Topology)) * prime;
            hash = (hash ^ std::hash<float>()(LineWidth)) * prime;
            hash = (hash ^ std::hash<bool>()(IsReverseCulling)) * prime;
            return hash;
        }
    };
    class GFXGraphicsPipeline
    {
    public:
        GFXGraphicsPipeline() = default;
        GFXGraphicsPipeline(const GFXGraphicsPipeline&) = delete;
        GFXGraphicsPipeline(GFXGraphicsPipeline&&) = delete;
        virtual ~GFXGraphicsPipeline() = default;
    public:

    };
}