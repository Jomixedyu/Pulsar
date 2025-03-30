#pragma once
#include "Pulsar/EngineMath.h"

namespace pulsar
{
    constexpr int STATICMESH_MAX_TEXTURE_COORDS = 4;

    struct StaticMeshVertex
    {
        Vector3f Position;
        Vector3f Normal;
        Vector3f Tangent;
        Vector3f Bitangent;
        Color4b Color;
        Vector2f TexCoords[STATICMESH_MAX_TEXTURE_COORDS];
    };
    constexpr inline int kSizeofStaticMeshVertex = sizeof(StaticMeshVertex);

    struct PerModelShaderParameter
    {
        Matrix4f LocalToWorldMatrix;
        Matrix4f WorldToLocalMatrix;
        Matrix4f NormalLocalToWorldMatrix;
        Vector4f NodePosition;
    };

    constexpr uint32_t kRenderingDescriptorSpace_ModelInfo = 2;
}