#include "Rendering/RenderObject.h"
#include "Rendering/PerRenderObjectDataManager.h"

namespace pulsar::rendering
{
    void RenderObject::SetTransform(const Matrix4f& localToWorld)
    {
        m_perRenderObjectData.LocalToWorldMatrix = localToWorld;
        m_perRenderObjectData.WorldToLocalMatrix = jmath::Inverse(localToWorld);

        m_perRenderObjectData.NormalLocalToWorldMatrix = jmath::Transpose(m_perRenderObjectData.WorldToLocalMatrix);
        m_isLocalToWorldDeterminantNegative = localToWorld.Determinant() < 0;

        if (m_renderObjectIndex != kInvalidSlot && m_pPerRenderObjectDataManager)
        {
            m_pPerRenderObjectDataManager->SetData(m_renderObjectIndex, m_perRenderObjectData);
        }

        OnChangedTransform();
    }
} // namespace pulsar::rendering
