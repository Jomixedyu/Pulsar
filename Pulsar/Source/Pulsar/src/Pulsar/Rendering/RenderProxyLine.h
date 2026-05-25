#pragma once
#include "RenderProxy.h"
#include "PrimitiveStruct.h"
#include <gfx/GFXHandle.h>

namespace pulsar
{
    class RenderProxyLine : public rendering::RenderProxy
    {
        using base = rendering::RenderProxy;

    private:
        array_list<StaticMeshVertex> m_verties;

        gfx::BufferHandle m_vertBuffer;
        gfx::GFXDescriptorSet_sp m_dummyExtraSet;
        gfx::GFXDescriptorSetLayout_sp m_meshDescriptorSetLayout;

        array_list<rendering::MeshBatch> m_batchs;
        bool m_depthTestEnabled = false;
        ShaderPassRenderQueueType m_renderQueue = ShaderPassRenderQueueType::Overlay;

    public:
        RenderProxyLine() = default;

        void SetPoints(const array_list<Vector3f>& pointPairs, const array_list<Color4b>& pointColors);
        void SetVerties(const array_list<StaticMeshVertex>& verties);
        size_t GetPointCount() const { return m_verties.size(); }
        void Fill();

        void SetDepthTestEnabled(bool enabled) { m_depthTestEnabled = enabled; }
        void SetQueue(ShaderPassRenderQueueType queue) { m_renderQueue = queue; }

        void InitRHI() override;
        void ReleaseRHI() override;

        void OnChangedTransform() override;
        array_list<rendering::MeshBatch> GetMeshBatches() override;
        std::string GetInterface() const override { return "RENDERER_STATICMESH"; }
    };

}