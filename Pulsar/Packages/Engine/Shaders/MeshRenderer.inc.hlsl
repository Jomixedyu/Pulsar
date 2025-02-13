#ifndef _MESH_RENDERER_INC
#define _MESH_RENDERER_INC

#include "Common.inc.hlsl"

struct PerObjectCBufferStruct
{
    float4x4 LocalToWorldMatrix;
    float4x4 WorldToLocalMatrix;
    float4x4 NormalLocalToWorldMatrix;
    float4   NodePosition;
    uint     ShaderFlags;
    uint     _Padding0;
    float2   _Padding1;
    float4   _Padding2;
    float4   _Padding3;
};
struct LightShaderParameter
{
    float4 WorldPosition;
    float4 Direction;
    float4 Color;
    float  SourceRadius;
    float  SoftSourceRadius;
    float  FalloffExponent;
    float  _Padding0;
    float2 SpotAngles;
    float2 _Padding1;
};

StructuredBuffer<LightShaderParameter> LightDataBuffer : register(b0, space2);

ConstantBuffer<PerObjectCBufferStruct> PerObjectBuffer : register(b0, space3);



#define USER_DESCSET space4

inline float4 ObjectToWorld(float3 position)
{
    return mul(PerObjectBuffer.LocalToWorldMatrix, float4(position, 1.0));
}
inline float4 ObjectToClip(float3 position)
{
    return mul(TargetBuffer.MatrixVP, ObjectToWorld(position));
}
inline float4 WorldToClip(float3 position)
{
    return mul(TargetBuffer.MatrixVP, float4(position, 1.f));
}
inline float3 ObjectNormalToWorld(float3 normal)
{
    return mul((float3x3)PerObjectBuffer.NormalLocalToWorldMatrix, normal);
}

#define PRIMITIVE_FLAGS_CAST_SHADOWS 0x1

#endif