#pragma once
#include "GFXInclude.h"
#include "GFXVertexLayoutDescription.h"

namespace gfx
{
    enum class GFXCullMode
    {
        None = 0,
        Front = 1,
        Back = 1 << 1,
        FrontAndBack = Front | Back,
    };
    enum class GFXBlendFactor
    {
        One,
        Zero,
        SrcColor,
        SrcAlpha,
        DstColor,
        DstAlpha,
        OneMinusSrcColor,
        OneMinusSrcAlpha,
        OneMinusDstColor,
        OneMinusDstAlpha
    };
    enum class GFXCompareMode
    {
        Never,
        Less,
        Equal,
        LessOrEqual,
        Greater,
        NotEqual,
        GreaterOrEqual,
        Always
    };
    enum class GFXStencilOp
    {
        Keep,
        Zero,
        Replace,
        IncrSat,
        DecrSat,
        Invert,
        IncrWrap,
        DecrWrap
    };

    enum class GFXPrimitiveTopology
    {
        TriangleList,
        LineList,
    };

    struct GFXGraphicsPipelineStateParams
    {
    public:
        bool DepthTestEnable;
        bool DepthWriteEnable;
        bool StencilTestEnable;

        GFXCullMode CullMode;
        GFXCompareMode DepthCompareOp;


        uint64_t GetHashCode() const
        {
            constexpr intptr_t HashS0 = 2166136261;
            constexpr intptr_t HashS1 = 16777619;
            uint64_t hashCode = HashS0;
            hashCode = hashCode * HashS1 ^ (uint64_t)CullMode;
            hashCode = hashCode * HashS1 ^ (uint64_t)DepthCompareOp;
            hashCode = hashCode * HashS1 ^ (uint64_t)DepthTestEnable;
            hashCode = hashCode * HashS1 ^ (uint64_t)DepthWriteEnable;
            hashCode = hashCode * HashS1 ^ (uint64_t)StencilTestEnable;

            return hashCode;
        }
    };

    struct GFXGraphicsPipelineState
    {
        GFXPrimitiveTopology Topology{};
        float LineWidth{1.0};
        array_list<GFXVertexLayoutDescription_sp> VertexLayouts;

        size_t GetHashCode() const
        {
            constexpr size_t prime = 16777619;
            size_t hash = 2166136261;
            hash = (hash ^ std::hash<GFXPrimitiveTopology>()(Topology)) * prime;
            hash = (hash ^ std::hash<float>()(LineWidth)) * prime;
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