#pragma once
#include <Pulsar/ObjectBase.h>
#include <gfx/GFXGraphicsPipeline.h>

namespace pulsar
{
    using CullMode = gfx::GFXCullMode;
    using CompareMode = gfx::GFXCompareMode;
    using StencilOp = gfx::GFXStencilOp;
    using BlendFactor = gfx::GFXBlendFactor;

    CORELIB_DEF_BOXING_ENUM(AssemblyObject_pulsar, pulsar, CullMode, "None = 0, Front = 1, Back = 2, FrontAndBack = 3");

    CORELIB_DEF_BOXING_ENUM(AssemblyObject_pulsar, pulsar, CompareMode, "Never,Less,Equal,LessOrEqual,Greater,NotEqual,GreaterOrEqual,Always");

    CORELIB_DEF_BOXING_ENUM(AssemblyObject_pulsar, pulsar, StencilOp, "Keep,Zero,Replace,IncrSat,DecrSat,Invert,IncrWrap,DecrWrap");
    CORELIB_DEF_BOXING_ENUM(AssemblyObject_pulsar, pulsar, BlendFactor, "One,Zero,SrcColor,SrcAlpha,DstColor,DstAlpha,OneMinusSrcColor,OneMinusSrcAlpha,OneMinusDstColor,OneMinusDstAlpha");


}

CORELIB_DECL_BOXING(pulsar::CullMode, pulsar::BoxingCullMode);
CORELIB_DECL_BOXING(pulsar::CompareMode, pulsar::BoxingCompareMode);
CORELIB_DECL_BOXING(pulsar::StencilOp, pulsar::BoxingStencilOp);
CORELIB_DECL_BOXING(pulsar::BlendFactor, pulsar::BoxingBlendFactor);