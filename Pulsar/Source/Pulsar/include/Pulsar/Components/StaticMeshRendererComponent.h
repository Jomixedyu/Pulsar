#pragma once

#include "Pulsar/Application.h"

#include "Component.h"
#include "RendererComponent.h"
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Assets/StaticMesh.h>
#include <Pulsar/Node.h>
#include <Pulsar/ObjectBase.h>

namespace pulsar
{
    class Mesh;
    class Material;
    class ShaderPass;
    class StaticMeshRenderObject;

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

        void OnReceiveMessage(MessageId id) override;

        int32_t GetRenderQueuePriority() const { return m_renderQueuePriority; }
        void SetRenderQueuePriority(int32_t value) { m_renderQueuePriority = value; }
    protected:
        void ResizeMaterials(size_t size);
        void BeginListenMaterialStateChanged(size_t index);
        void EndListenMaterialStateChanged(size_t index);
        void OnMaterialStateChanged();
        void OnMsg_TransformChanged() override;
        void OnMeshChanged();
        void OnMaterialChanged();
    protected:
        CORELIB_REFL_DECL_FIELD(m_materials, new ListItemAttribute(cltypeof<Material>()));
        List_sp<Material_ref> m_materials;
        size_t m_materialsSize = 0;
        array_list<uint32_t> m_materialStateChangedCallbacks;

        CORELIB_REFL_DECL_FIELD(m_staticMesh);
        StaticMesh_ref m_staticMesh;

        CORELIB_REFL_DECL_FIELD(m_isCastShadow);
        bool m_isCastShadow = true;

        CORELIB_REFL_DECL_FIELD(m_renderQueuePriority);
        int32_t m_renderQueuePriority{1000};

        sptr<StaticMeshRenderObject> m_renderObject;

    private:

    };
    DECL_PTR(StaticMeshRendererComponent);
}