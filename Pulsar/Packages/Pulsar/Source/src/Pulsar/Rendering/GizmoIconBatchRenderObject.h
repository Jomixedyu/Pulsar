#pragma once
#include "RenderObjectProxy.h"

#include <memory>

namespace pulsar
{
    namespace rendering { class MeshProxy; }
    class MaterialProxy;
    class GizmoIconBatchRenderObject : public rendering::RenderObjectProxy
    {
    public:
        struct IconItem
        {
            Matrix4f Transform;
            std::shared_ptr<MaterialProxy> MaterialProxy;
        };

        void SetMesh(const std::shared_ptr<rendering::MeshProxy>& mesh);
        void SetItems(const array_list<IconItem>& items);

        void OnEnterScene() override;
        void OnLeaveScene() override;
        void OnChangedTransform() override {}
        array_list<rendering::MeshBatch> GetMeshBatches() override;

    private:
        void _Rebuild();

        std::shared_ptr<rendering::MeshProxy> m_mesh;
        array_list<IconItem> m_items;
        bool m_dirty = true;

        gfx::GFXDescriptorSetLayoutPtr m_descriptorSetLayout;
        gfx::GFXDescriptorSetPtr m_dummyExtraSet;

        array_list<uint32_t> m_itemSlots;
        array_list<rendering::MeshBatch> m_batches;
    };
}
