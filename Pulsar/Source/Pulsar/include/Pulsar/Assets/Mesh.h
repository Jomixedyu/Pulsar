#pragma once
#include <Pulsar/AssetObject.h>
#include <Pulsar/IGPUResource.h>

namespace pulsar
{
    class Mesh : public AssetObject, public IGPUResource
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Mesh, AssetObject);
    public:
        virtual size_t GetVertexCount() = 0;
    };
}