#pragma once

#include <Pulsar/ObjectBase.h>
#include <Pulsar/Assets/Material.h>
#include "Component.h"


namespace pulsar
{
    class Mesh;
    class Material;
    class ShaderPass;

    class RendererComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::RendererComponent, Component)
    public:
        virtual void OnConstruct() override;
        List_sp<Material_sp> get_materials() const { return this->materials_; }
    protected:
        CORELIB_REFL_DECL_FIELD(materials_);
        List_sp<Material_sp> materials_;
    };
    CORELIB_DECL_SHORTSPTR(RendererComponent);
}