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
        Color4f Color;
        Vector2f TexCoords[STATICMESH_MAX_TEXTURE_COORDS];
    };
    constexpr inline int kSizeofStaticMeshVertex = sizeof(StaticMeshVertex);

    struct CBuffer_ModelObject
    {
        Matrix4f LocalToWorldMatrix;
    };

    constexpr uint32_t kRenderingDescriptorSpace_ModelInfo = 2;
}