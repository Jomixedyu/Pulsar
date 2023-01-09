#pragma once

#include <Apatite/ObjectBase.h>
#include "Component.h"
#include "RendererComponent.h"
#include <vector>

namespace apatite
{
    class Mesh;
    class Material;
    class ShaderProgram;

    class StaticMeshRendererComponent : public RendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::StaticMeshRendererComponent, RendererComponent)
    public:
        void set_material(Material_rsp value) { this->material_ = value; }
        Material_sp get_material() { return this->material_; }
    public:

    private:
        Material_sp material_;
    };
    CORELIB_DECL_SHORTSPTR(StaticMeshRendererComponent);
}