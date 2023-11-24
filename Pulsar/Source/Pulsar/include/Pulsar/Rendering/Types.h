#pragma once
#include <Pulsar/ObjectBase.h>
#include <gfx/GFXShaderPass.h>

namespace pulsar
{
    using CullMode = gfx::GFXCullMode;
    using CompareMode = gfx::GFXCompareMode;
    
    CORELIB_DEF_BOXING_ENUM(AssemblyObject_pulsar, pulsar, CullMode, "None = 0, Front = 1, Back = 2, FrontAndBack = 3");

    CORELIB_DEF_BOXING_ENUM(AssemblyObject_pulsar, pulsar, CompareMode, "Never,Less,Equal,LessOrEqual,Greater,NotEqual,GreaterOrEqual,Always");


}

CORELIB_DECL_BOXING(pulsar::CullMode, pulsar::BoxingCullMode);
CORELIB_DECL_BOXING(pulsar::CompareMode, pulsar::BoxingCompareMode);