�   {
	"PassName": "Engine/Missing",
	"CullMode": "None",
	"DepthTestEnable": true,
	"DepthWriteEnable": true,
	"DepthCompareOp": "Less",
	"StencilTestEnable": false
}J   (  #                     GLSL.std.450                                        D:/Codes/Pulsar/Pulsar/Shader/Common.inc.hlsl        D:/Codes/Pulsar/Pulsar/Shader/DefaultVS.inc.hlsl         �      �   �     #pragma once

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
#define EIS_VERTCOLOR LAYOUT_LOCATION(3)
#define EIS_TEXCOORD0 LAYOUT_LOCATION(4)
#define EIS_TEXCOORD1 LAYOUT_LOCATION(5)
#define EIS_TEXCOORD2 LAYOUT_LOCATION(6)
#define EIS_TEXCOORD3 LAYOUT_LOCATION(7)

struct InVertexAssembly
{
    EIS_POSITION  float3 Position  : POSITION;
    EIS_NORMAL    float3 Normal    : NORMAL;
    EIS_TANGENT   float3 Tangent   : TANGENT;
    EIS_VERTCOLOR float4 Color     : COLOR0;
    EIS_TEXCOORD0 float2 TexCoord0 : TEXCOORD0;
    EIS_TEXCOORD1 float2 TexCoord1 : TEXCOORD1;
    EIS_TEXCOORD2 float2 TexCoord2 : TEXCOORD2;
    EIS_TEXCOORD3 float2 TexCoord3 : TEXCOORD3;
};

struct InPixelAssembly
{
    float4 Position : SV_POSITION;
    float3 WorldNormal : NORMAL0;
    float4 Color : COLOR0;
    float2 TexCoord0 : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
    float2 TexCoord2 : TEXCOORD2;
    float2 TexCoord3 : TEXCOORD3;
};

struct OutPixelAssembly
{
    float4 Color : SV_TARGET;
};

cbuffer CB_Camera : register(b0)
{
    float4x4 _MatrixV;
    float4x4 _MatrixP;
    float4x4 _MatrixVP;
};
cbuffer CB_Lighting : register(b1)
{
    float4 _WorldSpaceLightPos;
    float3 _WorldSpaceLightColor;
};

cbuffer ShaderCBuffer : register(b2)
{
    uint _ShaderFlags;
    float4x4 _MatrixLocalToWorld;
    float3 _NodePosition;
}

inline float4 ObjectToWorld(in float3 position)
{
    return mul(_MatrixVP, mul(_MatrixLocalToWorld, float4(position, 1.0)));
}

#define PRIMITIVE_FLAGS_CAST_SHADOWS 0x1  y    �     #pragma once
#include "Common.inc.hlsl"

#define IMPL_DEFAULT_VERT \
InPixelAssembly main(InVertexAssembly a2v) \
{ \
    InPixelAssembly v2f = (InPixelAssembly) 0; \
    v2f.WorldNormal = a2v.Normal; \
    v2f.TexCoord0 = a2v.TexCoord0; \
    v2f.TexCoord1 = a2v.TexCoord1; \
    v2f.TexCoord2 = a2v.TexCoord2; \
    v2f.TexCoord3 = a2v.TexCoord3; \
    v2f.Color = a2v.Color; \
    v2f.Position = ObjectToWorld(a2v.Position); \
    return v2f; \
}                  CB_Camera            _MatrixV            _MatrixP            _MatrixVP                 CB_Lighting          _WorldSpaceLightPos  	       _WorldSpaceLightColor                 ShaderCBuffer            _ShaderFlags            _MatrixLocalToWorld         _NodePosition            J client vulkan100    J target-env spirv1.3 J target-env vulkan1.2    J hlsl-offsets    J client vulkan100    J target-env spirv1.3 J target-env vulkan1.2    J hlsl-offsets    H            H         #       H               H           H        #   @   H              H           H        #   �   H              G        G     "       G     !       H         #       H        #      G        G     "       G     !      H         #       H           H        #      H              H        #   P   G        G     "       G     !           !          	         
   	           
                                ;                	           
                  ;                                                  ;           6               �     �  8    #                     GLSL.std.450                                               D:/Codes/Pulsar/Pulsar/Shader/Common.inc.hlsl        �      �   �     #pragma once

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
#define EIS_VERTCOLOR LAYOUT_LOCATION(3)
#define EIS_TEXCOORD0 LAYOUT_LOCATION(4)
#define EIS_TEXCOORD1 LAYOUT_LOCATION(5)
#define EIS_TEXCOORD2 LAYOUT_LOCATION(6)
#define EIS_TEXCOORD3 LAYOUT_LOCATION(7)

struct InVertexAssembly
{
    EIS_POSITION  float3 Position  : POSITION;
    EIS_NORMAL    float3 Normal    : NORMAL;
    EIS_TANGENT   float3 Tangent   : TANGENT;
    EIS_VERTCOLOR float4 Color     : COLOR0;
    EIS_TEXCOORD0 float2 TexCoord0 : TEXCOORD0;
    EIS_TEXCOORD1 float2 TexCoord1 : TEXCOORD1;
    EIS_TEXCOORD2 float2 TexCoord2 : TEXCOORD2;
    EIS_TEXCOORD3 float2 TexCoord3 : TEXCOORD3;
};

struct InPixelAssembly
{
    float4 Position : SV_POSITION;
    float3 WorldNormal : NORMAL0;
    float4 Color : COLOR0;
    float2 TexCoord0 : TEXCOORD0;
    float2 TexCoord1 : TEXCOORD1;
    float2 TexCoord2 : TEXCOORD2;
    float2 TexCoord3 : TEXCOORD3;
};

struct OutPixelAssembly
{
    float4 Color : SV_TARGET;
};

cbuffer CB_Camera : register(b0)
{
    float4x4 _MatrixV;
    float4x4 _MatrixP;
    float4x4 _MatrixVP;
};
cbuffer CB_Lighting : register(b1)
{
    float4 _WorldSpaceLightPos;
    float3 _WorldSpaceLightColor;
};

cbuffer ShaderCBuffer : register(b2)
{
    uint _ShaderFlags;
    float4x4 _MatrixLocalToWorld;
    float3 _NodePosition;
}

inline float4 ObjectToWorld(in float3 position)
{
    return mul(_MatrixVP, mul(_MatrixLocalToWorld, float4(position, 1.0)));
}

#define PRIMITIVE_FLAGS_CAST_SHADOWS 0x1               CB_Camera            _MatrixV            _MatrixP            _MatrixVP                 CB_Lighting          _WorldSpaceLightPos  	       _WorldSpaceLightColor                 ShaderCBuffer            _ShaderFlags            _MatrixLocalToWorld         _NodePosition            J client vulkan100    J target-env spirv1.3 J target-env vulkan1.2    J hlsl-offsets    J client vulkan100    J target-env spirv1.3 J target-env vulkan1.2    J hlsl-offsets    H            H         #       H               H           H        #   @   H              H           H        #   �   H              G        G     "       G     !       H         #       H        #      G        G     "       G     !      H         #       H           H        #      H              H        #   P   G        G     "       G     !           !                   	           
   	           
   
   
               ;                           	                  ;                                
                  ;           6               �     �  8  