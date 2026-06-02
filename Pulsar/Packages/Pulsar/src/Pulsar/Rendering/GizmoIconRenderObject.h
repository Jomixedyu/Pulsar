#pragma once
#include "RenderObject.h"
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Assets/StaticMesh.h>

namespace pulsar
{
    class GizmoIconRenderObject : public rendering::RenderObject
    {
    public:
        void SetMesh(const RCPtr<StaticMesh>& mesh);
        void SetMaterial(const RCPtr<Material>& material);

        void OnCreateResource() override;
        void OnDestroyResource() override;
        void OnChangedTransform() override;
        array_list<rendering::MeshBatch> GetMeshBatches() override;

    private:
        void _RebuildBatch();

        RCPtr<StaticMesh> m_mesh;
        RCPtr<Material> m_material;
        gfx::GFXDescriptorSet_sp m_dummyExtraSet;
        gfx::GFXDescriptorSetLayout_sp m_descriptorSetLayout;
        array_list<rendering::MeshBatch> m_batches;
    };
}
