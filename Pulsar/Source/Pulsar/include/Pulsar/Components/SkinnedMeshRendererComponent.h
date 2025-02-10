#pragma once
#include "MeshRendererComponent.h"
#include "Assets/SkinnedMesh.h"

namespace pulsar
{
    class SkinnedMeshRendererComponent : public MeshRendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SkinnedMeshRendererComponent, MeshRendererComponent);
    public:

        void BeginComponent() override;
        void EndComponent() override;
        
    protected:
        CORELIB_REFL_DECL_FIELD(m_mesh);
        RCPtr<SkinnedMesh> m_mesh;
    };
}