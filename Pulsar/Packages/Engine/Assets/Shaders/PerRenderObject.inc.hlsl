#ifndef _ENGINE_PER_RENDER_OBJECT_INC
#define _ENGINE_PER_RENDER_OBJECT_INC

// Per-RenderObject Constants (set 1 / space1, binding 6)
// Bound per render object via dynamic UBO offset.

struct RenderObjectData
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

ConstantBuffer<RenderObjectData> RenderObjectBuffer : register(b6, space1);


#ifdef RENDERER_SKINNEDMESH
// GPU Skinning: 最多 256 根骨骼矩阵 (set2 binding1)
struct SkinnedRenderObjectData
{
    float4x4 BoneMatrices[256];
};
ConstantBuffer<SkinnedRenderObjectData> SkinningBuffer : register(b1, space2);
#endif // RENDERER_SKINNEDMESH

#endif // _ENGINE_PER_RENDER_OBJECT_INC
