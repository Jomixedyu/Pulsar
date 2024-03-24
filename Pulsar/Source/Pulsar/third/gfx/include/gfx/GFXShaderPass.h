#pragma once
#include "GFXInclude.h"
#include "GFXGpuProgram.h"
#include "GFXVertexLayoutDescription.h"
#include "GFXDescriptorSet.h"
#include "GFXRenderPass.h"

namespace gfx
{
    enum class GFXCullMode
    {
        None = 0,
        Front = 1,
        Back = 1 << 1,
        FrontAndBack = Front | Back,
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
    enum class GFXPrimitiveTopology
    {
        TriangleList,
        LineList,
    };

    struct GFXShaderPassConfig
    {
    public:
        GFXCullMode CullMode;
        bool DepthTestEnable;
        bool DepthWriteEnable;
        GFXCompareMode DepthCompareOp;
        bool StencilTestEnable;
    };

    class GFXShaderPass
    {
    public:
        virtual ~GFXShaderPass() = default;

        virtual GFXShaderPassConfig GetStateConfig() const = 0;
    };
    GFX_DECL_SPTR(GFXShaderPass);
}