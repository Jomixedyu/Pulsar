#pragma once
#include "Component.h"
#include <Apatite/Assets/StaticMesh.h>

namespace apatite
{
    class MeshContainerComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::MeshContainerComponent, Component);
    public:
        void set_mesh(StaticMesh_sp value) { this->mesh_ = value; }
        StaticMesh_sp get_mesh() { return this->mesh_; }
    private:
        StaticMesh_sp mesh_;
    };
    CORELIB_DECL_SHORTSPTR(MeshContainerComponent);
}