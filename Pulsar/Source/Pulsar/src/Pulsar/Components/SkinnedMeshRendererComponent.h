#pragma once

#include <Pulsar/Assets/SkinnedMesh.h>
#include "RendererComponent.h"
#include <Pulsar/Node.h>
#include <Pulsar/Assets/Material.h>

namespace pulsar
{
    class SkinnedMeshRenderObject;

    class SkinnedMeshRendererComponent : public RendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SkinnedMeshRendererComponent, RendererComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Renderer"));
    public:
        SkinnedMeshRendererComponent();

        SPtr<rendering::RenderObject> CreateRenderObject() override;

        void BeginComponent() override;
        void EndComponent() override;

        void PostEditChange(FieldInfo* info) override;
        void OnDrawGizmo(GizmoPainter* painter, bool selected) override;

        void SetSkinnedMesh(const RCPtr<SkinnedMesh>& mesh);
        RCPtr<SkinnedMesh> GetSkinnedMesh() const { return m_skinnedMesh; }

        RCPtr<Material> GetMaterial(int index) const;
        void            SetMaterial(int index, RCPtr<Material> material);
        size_t          AddMaterial(RCPtr<Material> material = nullptr);
        size_t          GetMaterialCount() const { return m_materialsSize; }

        // 每帧由 Animator 调用，更新骨骼矩阵到 GPU
        void UpdateBoneMatrices(const array_list<Matrix4f>& boneMatrices);

        bool HasBounds() const override { return false; }
        void GetSubscribeObserverHandles(array_list<ObjectHandle>& out) override;

    protected:
        void OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg) override;
        void ResizeMaterials(size_t size);
        void OnTransformChanged() override;
        void OnMeshChanged();
        void OnMaterialChanged();
        void GetDependenciesAsset(array_list<jxcorlib::guid_t> &deps) const override;
    protected:
        CORELIB_REFL_DECL_FIELD(m_materials, new ListItemAttribute(cltypeof<MaterialSlot>()));
        List_sp<SPtr<MaterialSlot>> m_materials;
        size_t m_materialsSize = 0;

        CORELIB_REFL_DECL_FIELD(m_skinnedMesh);
        RCPtr<SkinnedMesh> m_skinnedMesh;

        CORELIB_REFL_DECL_FIELD(m_isCastShadow);
        bool m_isCastShadow = true;

        SPtr<SkinnedMeshRenderObject> m_renderObject;
    };

}