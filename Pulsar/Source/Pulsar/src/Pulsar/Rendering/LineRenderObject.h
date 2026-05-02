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
        bool m_depthTestEnabled = false;
        ShaderPassRenderQueueType m_renderQueue = ShaderPassRenderQueueType::Overlay;

    public:
        LineRenderObject() = default;

        void SetPoints(const array_list<Vector3f>& pointPairs, const array_list<Color4b>& pointColors);
        void SetVerties(const array_list<StaticMeshVertex>& verties);
        size_t GetPointCount() const { return m_verties.size(); }
        void Fill();

        void SetDepthTestEnabled(bool enabled) { m_depthTestEnabled = enabled; }
        void SetQueue(ShaderPassRenderQueueType queue) { m_renderQueue = queue; }

        void OnCreateResource() override;
        void OnDestroyResource() override;

        void OnChangedTransform() override;
        array_list<rendering::MeshBatch> GetMeshBatches() override;
        std::string GetInterface() const override { return "RENDERER_STATICMESH"; }
    };

}