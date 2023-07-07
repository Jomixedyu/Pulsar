#pragma once
#include <Pulsar/AssetObject.h>

namespace pulsar
{
    class Mesh : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::Mesh, AssetObject);
    public:
        virtual size_t GetVertexCount() = 0;
    };
}