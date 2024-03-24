#pragma once
#include "RenderObject.h"
#include "PrimitiveStruct.h"

namespace pulsar
{
    class LineRenderObject : public rendering::RenderObject
    {
        using base = rendering::RenderObject;

    private:
        array_list<StaticMeshVertex> m_verties;

        gfx::GFXBuffer_sp m_vertBuffer;
        gfx::GFXBuffer_sp m_meshConstantBuffer;
        gfx::GFXDescriptorSet_sp m_meshObjDescriptorSet;
        gfx::GFXDescriptorSetLayout_sp m_meshDescriptorSetLayout;

        array_list<rendering::MeshBatch> m_batchs;

    public:
        LineRenderObject() = default;

        void SetPoints(const array_list<Vector3f>& pointPairs, const array_list<Color4f>& pointColors);
        void SetVerties(const array_list<StaticMeshVertex>& verties);
        void OnCreateResource() override;
        void OnDestroyResource() override
        {
            base::OnDestroyResource();
            m_vertBuffer.reset();

        }
        void OnChangedTransform() override
        {
            m_meshConstantBuffer->Fill(&m_perModelData);
        }
        array_list<rendering::MeshBatch> GetMeshBatchs() override
        {
            for (const auto& batch : m_batchs)
            {
                if (batch.Material && !batch.Material->IsCreatedGPUResource())
                {
                    batch.Material->CreateGPUResource();
                }
            }
            return m_batchs;
        }
    };

}