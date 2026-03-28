#pragma once
#include "Assets/SkinnedMesh.h"
#include "RendererComponent.h"

namespace pulsar
{
    class SkinnedMeshRendererComponent : public RendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SkinnedMeshRendererComponent, RendererComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Renderer"));
    public:
        SkinnedMeshRendererComponent();

        void BeginComponent() override;
        void EndComponent() override;

        void PostEditChange(FieldInfo* info) override;

        void OnDrawGizmo(GizmoPainter* painter, bool selected) override;

        void SetSkinnedMesh(const RCPtr<SkinnedMesh>& mesh);
        RCPtr<SkinnedMesh> GetSkinnedMesh() const { return m_skinnedMesh; }

    protected:
        CORELIB_REFL_DECL_FIELD(m_skinnedMesh);
        RCPtr<SkinnedMesh> m_skinnedMesh;
    };
}