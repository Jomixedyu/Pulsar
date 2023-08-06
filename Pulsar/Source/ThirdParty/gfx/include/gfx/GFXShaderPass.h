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
        virtual ~GFXShaderPass() {}

        virtual std::shared_ptr<GFXDescriptorSetLayout> GetDescriptorSetLayout() const = 0;
        virtual std::shared_ptr<GFXVertexLayoutDescription> GetVertexLayout() const = 0;
        virtual GFXShaderPassConfig GetStateConfig() const = 0;
    };
    GFX_DECL_SPTR(GFXShaderPass);
}