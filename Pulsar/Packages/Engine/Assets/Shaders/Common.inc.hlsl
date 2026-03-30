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
#define EIS_POSITION  LAYOUT_LOCATION(0)
#define EIS_NORMAL    LAYOUT_LOCATION(1)
#define EIS_TANGENT   LAYOUT_LOCATION(2)
#define EIS_BITANGENT LAYOUT_LOCATION(3)
#define EIS_VERTCOLOR LAYOUT_LOCATION(4)
#define EIS_TEXCOORD0 LAYOUT_LOCATION(5)
#define EIS_TEXCOORD1 LAYOUT_LOCATION(6)
#define EIS_TEXCOORD2 LAYOUT_LOCATION(7)
#define EIS_TEXCOORD3 LAYOUT_LOCATION(8)

struct StandardAttributes
{
    EIS_POSITION  float3 Position  : POSITION;
    EIS_NORMAL    float3 Normal    : NORMAL;
    EIS_TANGENT   float3 Tangent   : TANGENT;
    EIS_BITANGENT float3 BiTangent : BINORMAL;
    EIS_VERTCOLOR float4 Color     : COLOR0;
    EIS_TEXCOORD0 float2 TexCoord0 : TEXCOORD0;
    EIS_TEXCOORD1 float2 TexCoord1 : TEXCOORD1;
    EIS_TEXCOORD2 float2 TexCoord2 : TEXCOORD2;
    EIS_TEXCOORD3 float2 TexCoord3 : TEXCOORD3;
};

struct StandardVaryings
{
    float4 Position : SV_POSITION;
    float4 WorldPosition : POSITION;
    float3 WorldNormal : NORMAL0;
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
