#pragma once
#include "Pulsar/EngineMath.h"

namespace pulsar
{
    constexpr int STATICMESH_MAX_TEXTURE_COORDS = 8;
    constexpr int SKINNEDMESH_MAX_BONE_INFLUENCES = 4;
    constexpr int SKINNEDMESH_MAX_BONES = 256;

    // GPU 上传时使用的临时交错结构，不用于序列化
    struct StaticMeshVertex
    {
        Vector3f Position;
        Vector3f Normal;
        Vector4f Tangent;   // xyz=切线方向，w=副切线符号(+1/-1)
        Color4b  Color;
        Vector2f TexCoords[STATICMESH_MAX_TEXTURE_COORDS];
    };
    constexpr inline int kSizeofStaticMeshVertex = sizeof(StaticMeshVertex);

    struct PerRendererData
    {
        Matrix4f LocalToWorldMatrix;
        Matrix4f WorldToLocalMatrix;
        Matrix4f NormalLocalToWorldMatrix;
        Vector4f NodePosition;
        uint32_t ShaderFlags;
        uint32_t _Padding0;
        Vector2f _Padding1;
        Vector4f _Padding2;
        Vector4f _Padding3;
    };

    // GPU 上传时使用的临时交错结构，不用于序列化
    struct SkinnedMeshVertex
    {
        Vector3f Position;
        Vector3f Normal;
        Vector4f Tangent;   // xyz=切线方向，w=副切线符号(+1/-1)
        Color4b  Color;
        Vector2f TexCoords[STATICMESH_MAX_TEXTURE_COORDS];
        uint8_t  BoneIndices[SKINNEDMESH_MAX_BONE_INFLUENCES]; // 骨骼索引（最多256根骨骼）
        float    BoneWeights[SKINNEDMESH_MAX_BONE_INFLUENCES]; // 骨骼权重（归一化，和为1）
    };
    constexpr inline int kSizeofSkinnedMeshVertex = sizeof(SkinnedMeshVertex);

    // set2 binding1：GPU Skinning 骨骼矩阵 UBO
    struct SkinnedRendererData
    {
        Matrix4f BoneMatrices[SKINNEDMESH_MAX_BONES];
    };

    constexpr uint32_t kRenderingDescriptorSpace_ModelInfo = 2;
    constexpr uint32_t kRenderingDescriptorBinding_PerRenderer   = 0; // PerRendererData
    constexpr uint32_t kRenderingDescriptorBinding_SkinningData  = 1; // SkinnedRendererData
}
