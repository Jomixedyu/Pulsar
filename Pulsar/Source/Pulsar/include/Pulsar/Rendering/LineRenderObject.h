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
        size_t GetPointCount() const { return m_verties.size(); }
        void Fill();

        void OnCreateResource() override;
        void OnDestroyResource() override;

        void OnChangedTransform() override;
        array_list<rendering::MeshBatch> GetMeshBatchs() override;
    };

}