#pragma once
#include "Pulsar/EngineMath.h"

namespace pulsar
{
    constexpr int STATICMESH_MAX_TEXTURE_COORDS = 8;

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

    constexpr uint32_t kRenderingDescriptorSpace_ModelInfo = 2;
}