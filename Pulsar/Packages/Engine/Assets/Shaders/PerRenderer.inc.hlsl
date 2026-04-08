#ifndef _ENGINE_PER_RENDERER_INC
#define _ENGINE_PER_RENDERER_INC

// Per-Renderer Constants (set 2 / space2)
// Bound per render object / draw call.

struct RendererData
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

ConstantBuffer<RendererData> RendererBuffer : register(b0, space2);


#ifdef RENDERER_SKINNEDMESH
// GPU Skinning: 最多 256 根骨骼矩阵 (set2 binding1)
struct SkinnedRendererData
{
    float4x4 BoneMatrices[256];
};
ConstantBuffer<SkinnedRendererData> SkinningBuffer : register(b1, space2);
#endif // RENDERER_SKINNEDMESH

#endif // _ENGINE_PER_RENDERER_INC
