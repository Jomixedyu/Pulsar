#pragma once
#include <Pulsar/EngineMath.h>
#include <Pulsar/Assets/StaticMesh.h>

namespace pulsar
{
    enum class GizmosDrawType
    {
        Lines,
        Image
    };
    class Gizmos
    {
    public:
        Matrix4f WorldMatrix;
        array_list<StaticMeshVertex> Points;
        GizmosDrawType DrawType;

    };

    class GizmosManager
    {
    public:

    };

}