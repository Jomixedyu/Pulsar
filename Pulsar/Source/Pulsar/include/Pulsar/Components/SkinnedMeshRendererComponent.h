#pragma once
#include "Assets/SkinnedMesh.h"
#include "MeshRendererComponent.h"
#include "RendererComponent.h"

namespace pulsar
{
    class SkinnedMeshRendererComponent : public MeshRendererComponent, public IRendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SkinnedMeshRendererComponent, MeshRendererComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Renderer"));
        CORELIB_IMPL_INTERFACES(IRendererComponent);
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