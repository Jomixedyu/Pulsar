#pragma once
#include "PropertyControl.h"
#include <Pulsar/EngineMath.h>

namespace pulsared
{
    class Mask8PropertyControl : public PropertyControl
    {
        EDITOR_IMPL_PROPERTY_CONTROL(cltypeof<BoxingMask8>(), new Mask8PropertyControl);
    public:

        virtual bool OnDrawImGui(const string& name, Type* type, Object* prop) override;

    };
}