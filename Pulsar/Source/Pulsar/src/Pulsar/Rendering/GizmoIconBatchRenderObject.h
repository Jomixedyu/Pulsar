#pragma once
#include "RenderObject.h"
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Assets/StaticMesh.h>

namespace pulsar
{
    class GizmoIconBatchRenderObject : public rendering::RenderObject
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

        struct PerItemGPU
        {
            gfx::GFXBuffer_sp ConstantBuffer;
            gfx::GFXDescriptorSet_sp DescriptorSet;
        };
        array_list<PerItemGPU> m_itemGPU;
        array_list<rendering::MeshBatch> m_batches;
    };
}
