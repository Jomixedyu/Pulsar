#pragma once

#include <Pulsar/ObjectBase.h>
#include "Component.h"
#include "RendererComponent.h"
#include <Pulsar/Node.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Assets/StaticMesh.h>

namespace pulsar
{
    class Mesh;
    class Material;
    class ShaderPass;

    class StaticMeshRendererComponent : public Component, public IRendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::StaticMeshRendererComponent, Component);
        CORELIB_IMPL_INTERFACES(IRendererComponent);
    public:
        virtual sptr<rendering::RenderObject> CreateRenderObject() override;
    public:
        virtual void OnInitialize() override;
        List_sp<Material_ref> GetMaterials() const { return this->m_materials; }

        StaticMeshRendererComponent() : CORELIB_INIT_INTERFACE(IRendererComponent)
        {
            m_materials = mksptr(new List<Material_ref>);
        }
    protected:
        CORELIB_REFL_DECL_FIELD(m_materials);
        List_sp<Material_ref> m_materials;

        CORELIB_REFL_DECL_FIELD(m_staticMesh);
        StaticMesh_ref m_staticMesh;

        bool m_isCastShadow;
    };
    DECL_PTR(StaticMeshRendererComponent);
}