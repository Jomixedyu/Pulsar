#pragma once
#include <Pulsar/IconsForkAwesome.h>
#include "Assets/TextureCube.h"
#include "Component.h"
#include "TextureCubeCaptureComponent.h"

namespace pulsar
{

    class IBakeComponent
    {
    public:
        virtual ~IBakeComponent() = default;
        virtual void Bake() {}
    };



    class ReflectionProbeComponent : public TextureCubeCaptureComponent, public IBakeComponent
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ReflectionProbeComponent, TextureCubeCaptureComponent);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Reflection"), new ComponentIconAttribute(ICON_FK_EYE));

    public:
        void RenderToTextureCube(RCPtr<TextureCube> cube);


    };
} // namespace pulsar