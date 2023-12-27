#include "Rendering/RenderObject.h"

namespace pulsar::rendering
{
    void RenderObject::SetTransform(const Matrix4f& localToWorld)
    {
        m_perModelData.LocalToWorldMatrix = localToWorld;
        m_perModelData.WorldToLocalMatrix = jmath::Inverse(localToWorld);

        // auto normalMat = localToWorld;
        // normalMat[3] = Vector4f{0,0,0,1};
        // m_perModelData.NormalLocalToWorldMatrix = jmath::Transpose(jmath::Inverse(normalMat));

        m_perModelData.NormalLocalToWorldMatrix = jmath::Transpose( m_perModelData.WorldToLocalMatrix);
        m_isLocalToWorldDeterminantNegative = localToWorld.Determinant() < 0;

        OnChangedTransform();
    }
} // namespace pulsar