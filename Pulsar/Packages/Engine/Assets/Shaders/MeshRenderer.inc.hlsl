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

float3 TransformTangentToWorld(float3 dirTS, float3 normalWS, float4 tangentWS)
{
    float3 T = tangentWS.xyz;
    float3 B = cross(normalWS, T) * tangentWS.w;
    float3x3 TBN = float3x3(T, B, normalWS);
    return mul(dirTS, TBN);
}

#define PRIMITIVE_FLAGS_CAST_SHADOWS 0x1

#ifdef RENDERER_SKINNEDMESH
// 将顶点位置从 BindPose 空间变换到蒙皮后的局部空间
inline float3 SkinPosition(float3 position, uint4 boneIndices, float4 boneWeights)
{
    float4 pos = float4(position, 1.0);
    float3 result = float3(0, 0, 0);
    result += boneWeights.x * mul(SkinningBuffer.BoneMatrices[boneIndices.x], pos).xyz;
    result += boneWeights.y * mul(SkinningBuffer.BoneMatrices[boneIndices.y], pos).xyz;
    result += boneWeights.z * mul(SkinningBuffer.BoneMatrices[boneIndices.z], pos).xyz;
    result += boneWeights.w * mul(SkinningBuffer.BoneMatrices[boneIndices.w], pos).xyz;
    return result;
}
// 将法线从 BindPose 空间变换到蒙皮后的局部空间（不含平移，用 float3x3）
inline float3 SkinNormal(float3 normal, uint4 boneIndices, float4 boneWeights)
{
    float3 result = float3(0, 0, 0);
    result += boneWeights.x * mul((float3x3)SkinningBuffer.BoneMatrices[boneIndices.x], normal);
    result += boneWeights.y * mul((float3x3)SkinningBuffer.BoneMatrices[boneIndices.y], normal);
    result += boneWeights.z * mul((float3x3)SkinningBuffer.BoneMatrices[boneIndices.z], normal);
    result += boneWeights.w * mul((float3x3)SkinningBuffer.BoneMatrices[boneIndices.w], normal);
    return normalize(result);
}
#endif // RENDERER_SKINNEDMESH

#endif