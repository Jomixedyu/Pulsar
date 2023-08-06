#pragma once
#include "Component.h"
#include <Pulsar/Assets/StaticMesh.h>

namespace pulsar
{
    class MeshContainerComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::MeshContainerComponent, Component);
    public:
        static MessageId MSG_MeshUpdate();
    public:
        void SetMesh(StaticMesh_sp value);
        StaticMesh_sp GetMesh() const;
    private:
        StaticMesh_sp m_mesh;
    };
    CORELIB_DECL_SHORTSPTR(MeshContainerComponent);
}