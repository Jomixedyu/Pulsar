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
    class ShaderConfig;
    class StaticMeshRenderObject;



    class StaticMeshRendererComponent : public RendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::StaticMeshRendererComponent, RendererComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Renderer"));
        DECL_OBJECTPTR_SELF
    public:
        SPtr<rendering::RenderObject> CreateRenderObject() override;
    public:
        void GetSubscribeObserverHandles(array_list<ObjectHandle>& out) override;
        List_sp<RCPtr<Material>> GetMaterials() const { return this->m_materials; }

        void PostEditChange(FieldInfo* info) override;

        StaticMeshRendererComponent();

        bool HasBounds() const override { return true; }
        BoxSphereBounds3f GetBoundsWS() override;

        RCPtr<StaticMesh> GetStaticMesh() const { return m_staticMesh; }
        void SetStaticMesh(RCPtr<StaticMesh> staticMesh);

        RCPtr<Material> GetMaterial(int index) const;
        void   SetMaterial(int index, RCPtr<Material> material);
        size_t AddMaterial(RCPtr<Material> material = nullptr);
        void   RemoveMaterial(size_t index);
        size_t GetMaterialCount() const { return m_materialsSize; }

        void BeginComponent() override;
        void EndComponent() override;
        void OnDrawGizmo(GizmoPainter* painter, bool selected) override;

        void OnReceiveMessage(MessageId id) override;

        bool GetShowNormalGizmo() const { return m_showNormalGizmo; }
        void SetShowNormalGizmo(bool v) { m_showNormalGizmo = v; }
        bool GetShowTangentGizmo() const { return m_showTangentGizmo; }
        void SetShowTangentGizmo(bool v) { m_showTangentGizmo = v; }
        bool GetShowBitangentGizmo() const { return m_showBitangentGizmo; }
        void SetShowBitangentGizmo(bool v) { m_showBitangentGizmo = v; }

    protected:
        void OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg) override;
        void ResizeMaterials(size_t size);
        // void BeginListenMaterialStateChanged(size_t index);
        // void EndListenMaterialStateChanged(size_t index);
        void OnMaterialStateChanged();
        void OnTransformChanged() override;
        void OnMeshChanged();
        void OnMaterialChanged();
    protected:
        CORELIB_REFL_DECL_FIELD(m_materials, new ListItemAttribute(cltypeof<Material>()));
        List_sp<RCPtr<Material>> m_materials;

        CORELIB_REFL_DECL_FIELD(m_priorities);
        List_sp<int32_t> m_priorities;

        size_t m_materialsSize = 0;

        CORELIB_REFL_DECL_FIELD(m_staticMesh);
        RCPtr<StaticMesh> m_staticMesh;

        CORELIB_REFL_DECL_FIELD(m_isCastShadow);
        bool m_isCastShadow = true;

        CORELIB_REFL_DECL_FIELD(m_boundsScale, new RangePropertyAttribute(0.1f, 10.f));
        float m_boundsScale = 1;

        CORELIB_REFL_DECL_FIELD(m_showNormalGizmo);
        bool m_showNormalGizmo = false;

        CORELIB_REFL_DECL_FIELD(m_showTangentGizmo);
        bool m_showTangentGizmo = false;

        CORELIB_REFL_DECL_FIELD(m_showBitangentGizmo);
        bool m_showBitangentGizmo = false;

        SPtr<StaticMeshRenderObject> m_renderObject;

    private:

    };
    DECL_PTR(StaticMeshRendererComponent);
}