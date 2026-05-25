#include "Rendering/RenderProxy.h"
#include "Rendering/RenderThread.h"
#include <Pulsar/Rendering/PerRenderObjectDataManager.h>

namespace pulsar::rendering
{
    void RenderProxy::SetTransform(const Matrix4f& localToWorld)
    {
        m_perRenderObjectData.LocalToWorldMatrix = localToWorld;
        m_perRenderObjectData.WorldToLocalMatrix = jmath::Inverse(localToWorld);

        m_perRenderObjectData.NormalLocalToWorldMatrix = jmath::Transpose(m_perRenderObjectData.WorldToLocalMatrix);
        m_isLocalToWorldDeterminantNegative = localToWorld.Determinant() < 0;

        // Notify subclasses (Game Thread side)
        OnChangedTransform();

        // Upload to GPU buffer via Render Thread
        auto index = m_renderObjectIndex;
        auto data = m_perRenderObjectData;
        RenderThread::Get().EnqueueCommand([index, data]() {
            if (index != kInvalidSlot)
            {
                RenderThread::Get().GetPerObjectDataManager().SetData(index, data);
            }
        });
    }
} // namespace pulsar::rendering
