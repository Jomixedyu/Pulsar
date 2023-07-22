#pragma once

#include <Pulsar/ObjectBase.h>
#include "Component.h"
#include "RendererComponent.h"
#include <Pulsar/Node.h>
#include <Pulsar/Assets/Material.h>
#include "Assets/StaticMesh.h"

namespace pulsar
{
    class Mesh;
    class Material;
    class ShaderPass;

    class StaticMeshRendererComponent : public RendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::StaticMeshRendererComponent, RendererComponent)
    public:
        virtual sptr<IRenderObject> CreateRenderObject() override;
    public:
        virtual void OnInitialize() override;
        List_sp<Material_sp> GetMaterials() const { return this->m_materials; }

    protected:
        CORELIB_REFL_DECL_FIELD(m_materials);
        List_sp<Material_sp> m_materials;

        CORELIB_REFL_DECL_FIELD(m_staticMesh);
        StaticMesh_sp m_staticMesh;
    };
    CORELIB_DECL_SHORTSPTR(StaticMeshRendererComponent);
}