#pragma once

#include <Pulsar/ObjectBase.h>
#include "Component.h"
#include "RendererComponent.h"
#include <Pulsar/Node.h>
#include <Pulsar/Assets/Material.h>

namespace pulsar
{
    class Mesh;
    class Material;
    class ShaderPass;

    class StaticMeshRendererComponent : public RendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::StaticMeshRendererComponent, RendererComponent)
    public:

    public:
        virtual void OnInitialize() override;
        virtual void OnDraw() override;

    };
    CORELIB_DECL_SHORTSPTR(StaticMeshRendererComponent);
}