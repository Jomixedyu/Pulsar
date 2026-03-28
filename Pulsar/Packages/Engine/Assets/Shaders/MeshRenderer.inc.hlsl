#ifndef _MESH_RENDERER_INC
#define _MESH_RENDERER_INC

#include "Common.inc.hlsl"
#include "PerRenderer.inc.hlsl"

inline float4 TransformObjectToWorld(float3 position)
{
    return mul(RendererBuffer.LocalToWorldMatrix, float4(position, 1.0));
}
inline float4 TransformObjectToClip(float3 position)
{
    return mul(CameraBuffer.MatrixVP, TransformObjectToWorld(position));
}
inline float4 TransformWorldToClip(float3 position)
{
    return mul(CameraBuffer.MatrixVP, float4(position, 1.f));
}
inline float3 TransformObjectNormalToWorld(float3 normal)
{
    return mul((float3x3)RendererBuffer.NormalLocalToWorldMatrix, normal);
}

#define PRIMITIVE_FLAGS_CAST_SHADOWS 0x1

#endif