#include "Components/SkinnedMeshRendererComponent.h"

namespace pulsar
{

    SkinnedMeshRendererComponent::SkinnedMeshRendererComponent()
        : CORELIB_INIT_INTERFACE(IRendererComponent)
    {
        m_canDrawGizmo = true;
    }

    void SkinnedMeshRendererComponent::BeginComponent()
    {
        base::BeginComponent();

    }
    void SkinnedMeshRendererComponent::EndComponent()
    {
        base::EndComponent();
    }
    void SkinnedMeshRendererComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        if (info->GetName() == NAMEOF(m_skinnedMesh))
        {
            SetSkinnedMesh(m_skinnedMesh);
        }
    }
    void SkinnedMeshRendererComponent::OnDrawGizmo(GizmoPainter* painter, bool selected)
    {
        base::OnDrawGizmo(painter, selected);

    }
    void SkinnedMeshRendererComponent::SetSkinnedMesh(const RCPtr<SkinnedMesh>& mesh)
    {
        if (m_skinnedMesh == mesh)
        {
            return;
        }

        m_skinnedMesh = mesh;
        // recreate render object

    }
} // namespace pulsar