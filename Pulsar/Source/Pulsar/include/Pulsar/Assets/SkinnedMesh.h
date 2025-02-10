#pragma once

#include "Mesh.h"

namespace pulsar
{
    class SkinnedMesh : public Mesh
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SkinnedMesh, Mesh);
    public:
        size_t GetVertexCount() override { throw NotImplementException(); }
        bool IsCreatedGPUResource() const override { throw NotImplementException(); }
        bool CreateGPUResource() override { throw NotImplementException(); }
        void DestroyGPUResource() override { throw NotImplementException(); }

    protected:

    };
} // namespace pulsar