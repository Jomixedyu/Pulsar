#ifndef _ENGINE_COMMON_INC
#define _ENGINE_COMMON_INC

#define PLATFORM_VULKAN

#ifdef PLATFORM_VULKAN
#define LAYOUT_LOCATION(index) [[vk::location(index)]]
#define VK_SAMPLER(index) [[vk::combinedImageSampler]][[vk::binding(index)]]
#else
#define LAYOUT_LOCATION(index)  
#endif

// Engine Input Semantic
// location 编号与 C++ EngineInputSemantic 枚举严格对应
#define EIS_POSITION  LAYOUT_LOCATION(0)
#define EIS_NORMAL    LAYOUT_LOCATION(1)
#define EIS_TANGENT   LAYOUT_LOCATION(2)
// location 3 保留（原 BITANGENT，已移除，w 分量并入 Tangent）
#define EIS_VERTCOLOR LAYOUT_LOCATION(4)
#define EIS_TEXCOORD0      LAYOUT_LOCATION(5)
#define EIS_TEXCOORD1      LAYOUT_LOCATION(6)
#define EIS_TEXCOORD2      LAYOUT_LOCATION(7)
#define EIS_TEXCOORD3      LAYOUT_LOCATION(8)
// location 9-12 保留给 TEXCOORD4-7
#define EIS_BLENDINDICES   LAYOUT_LOCATION(13)
#define EIS_BLENDWEIGHT    LAYOUT_LOCATION(14)

struct StandardAttributes
{
    EIS_POSITION  float3 Position  : POSITION;
    EIS_NORMAL    float3 Normal    : NORMAL;
    EIS_TANGENT   float4 Tangent   : TANGENT;   // xyz=切线方向, w=副切线符号(+1/-1)
    EIS_VERTCOLOR float4 Color     : COLOR0;
    EIS_TEXCOORD0 float2 TexCoord0 : TEXCOORD0;
    EIS_TEXCOORD1 float2 TexCoord1 : TEXCOORD1;
    EIS_TEXCOORD2 float2 TexCoord2 : TEXCOORD2;
    EIS_TEXCOORD3 float2 TexCoord3 : TEXCOORD3;
#ifdef RENDERER_SKINNEDMESH
    EIS_BLENDINDICES uint4  BlendIndices : BLENDINDICES;
    EIS_BLENDWEIGHT  float4 BlendWeights : BLENDWEIGHT;
#endif
};

struct StandardVaryings
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : POSITION;
    float3 WorldNormal : NORMAL0;
    float4 WorldTangent : TANGENT0;
    float4 Color : COLOR0;
    float2 TexCoord0 : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
    float2 TexCoord2 : TEXCOORD2;
    float2 TexCoord3 : TEXCOORD3;
};

// Descriptor Set Layout:
//   set 0 (space0) = per-Material  (user params, auto-assigned by compiler)
//   set 1 (space1) = per-Pass      (camera, world, lights)
//   set 2 (space2) = per-Renderer  (model matrix, bounds)

#include "PerPass.inc.hlsl"

#endif //_ENGINE_COMMON_INC
