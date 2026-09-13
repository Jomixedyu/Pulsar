#pragma once
#include "RenderObject.h"
#include "PrimitiveStruct.h"
#include <gfx/GFXBuffer.h>

namespace pulsar
{
    class LineRenderObject : public rendering::RenderObject
    {
        using base = rendering::RenderObject;

    private:
        array_list<StaticMeshVertex> m_verties;

        gfx::GFXBufferPtr m_vertBuffer;
        gfx::GFXDescriptorSetPtr m_dummyExtraSet;
        gfx::GFXDescriptorSetLayoutPtr m_meshDescriptorSetLayout;

        array_list<rendering::MeshBatch> m_batchs;
        RCPtr<Material> m_material;
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
    };

}
