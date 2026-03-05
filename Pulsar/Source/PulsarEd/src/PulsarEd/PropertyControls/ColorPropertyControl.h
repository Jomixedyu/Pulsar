#pragma once
#include "PropertyControl.h"
#include <Pulsar/EngineMath.h>

namespace pulsared
{
    class ColorPropertyControl : public PropertyControl
    {
        EDITOR_IMPL_PROPERTY_CONTROL(cltypeof<jxcorlib::math::BoxingColor4f>(), new ColorPropertyControl);
    public:
        virtual bool OnDrawImGui(const string& name, Type* type, Object* prop) override;

    };

}