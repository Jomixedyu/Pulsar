#pragma once
#include "RenderProxy.h"
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Assets/StaticMesh.h>

namespace pulsar
{
    class GizmoIconBatchRenderObject : public rendering::RenderProxy
    {
    public:
        struct IconItem
        {
            Matrix4f Transform;
            RCPtr<Material> Material;
        };

        void SetMesh(const RCPtr<StaticMesh>& mesh);
        void SetItems(const array_list<IconItem>& items);

        void OnCreateResource() override;
        void OnDestroyResource() override;
        void OnChangedTransform() override {}
        array_list<rendering::MeshBatch> GetMeshBatches() override;

    private:
        void _Rebuild();

        RCPtr<StaticMesh> m_mesh;
        array_list<IconItem> m_items;
        bool m_dirty = true;

        gfx::GFXDescriptorSetLayout_sp m_descriptorSetLayout;
        gfx::GFXDescriptorSet_sp m_dummyExtraSet;

        array_list<uint32_t> m_itemSlots;
        array_list<rendering::MeshBatch> m_batches;
    };
}
