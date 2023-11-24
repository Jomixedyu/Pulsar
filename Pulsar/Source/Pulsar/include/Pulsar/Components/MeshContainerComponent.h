#pragma once
#include "Component.h"
#include <Pulsar/Assets/StaticMesh.h>

namespace pulsar
{
    class MeshContainerComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::MeshContainerComponent, Component);
    public:
        static MessageId MSG_MeshUpdate();
    public:
        void SetMesh(StaticMesh_ref value);
        StaticMesh_ref GetMesh() const;
    private:
        CORELIB_REFL_DECL_FIELD(m_mesh);
        StaticMesh_ref m_mesh;
    };
    DECL_PTR(MeshContainerComponent);
}