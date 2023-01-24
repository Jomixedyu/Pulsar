#pragma once

#include <Apatite/ObjectBase.h>
#include "Component.h"
#include "RendererComponent.h"
#include <Apatite/Node.h>
#include <Apatite/Assets/Material.h>

namespace apatite
{
    class Mesh;
    class Material;
    class ShaderPass;

    class StaticMeshRendererComponent : public RendererComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::StaticMeshRendererComponent, RendererComponent)
    public:

    public:
        virtual void OnInitialize() override;
        virtual void OnDraw() override;

    };
    CORELIB_DECL_SHORTSPTR(StaticMeshRendererComponent);
}