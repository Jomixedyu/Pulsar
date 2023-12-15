#pragma once
#include "RenderObject.h"
#include "PrimitiveStruct.h"

namespace pulsar
{
    class LineRenderObject : public rendering::RenderObject
    {
        using base = rendering::RenderObject;

    public:
        array_list<Vector3f> m_pointPairs;
        array_list<Color4f> m_pointColors;

        array_list<StaticMeshVertex> m_verties;

        gfx::GFXBuffer_sp m_vertBuffer;
        gfx::GFXBuffer_sp m_meshConstantBuffer;
        gfx::GFXDescriptorSet_sp m_meshObjDescriptorSet;
        gfx::GFXDescriptorSetLayout_sp m_meshDescriptorSetLayout;

        array_list<rendering::MeshBatch> m_batchs;

    public:
        LineRenderObject() = default;

        static inline gfx::GFXDescriptorSetLayout_wp MeshDescriptorSetLayout{};
        void OnCreateResource() override;
        void OnDestroyResource() override
        {
            base::OnDestroyResource();
            m_vertBuffer.reset();
        }
        void OnChangedTransform() override
        {
            m_meshConstantBuffer->Fill(&m_localToWorld);
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