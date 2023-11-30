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
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::StaticMeshRendererComponent, Component);
        CORELIB_IMPL_INTERFACES(IRendererComponent);
    public:
        sptr<rendering::RenderObject> CreateRenderObject() override;
    public:
        List_sp<Material_ref> GetMaterials() const { return this->m_materials; }

        void PostEditChange(FieldInfo* info) override;

        StaticMeshRendererComponent() : CORELIB_INIT_INTERFACE(IRendererComponent)
        {
            m_materials = mksptr(new List<Material_ref>);
        }

        StaticMesh_ref GetStaticMesh() const { return m_staticMesh; }
        void SetStaticMesh(StaticMesh_ref staticMesh);

        Material_ref GetMaterial(int index) const;
        void SetMaterial(int index, Material_ref material);

        void BeginComponent() override;
        void EndComponent() override;
    protected:
        void OnMeshChanged();
    protected:
        CORELIB_REFL_DECL_FIELD(m_materials, new ListItemAttribute(cltypeof<Material>()));
        List_sp<Material_ref> m_materials;

        CORELIB_REFL_DECL_FIELD(m_staticMesh);
        StaticMesh_ref m_staticMesh;

        CORELIB_REFL_DECL_FIELD(m_isCastShadow);
        bool m_isCastShadow = true;

        sptr<rendering::RenderObject> m_renderObject;
    };
    DECL_PTR(StaticMeshRendererComponent);
}