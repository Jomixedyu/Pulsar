#pragma once
#include "PropertyControl.h"
#include <Pulsar/EngineMath.h>

namespace pulsared
{
    class Quat4fPropertyControl : public PropertyControl
    {
        EDITOR_IMPL_PROPERTY_CONTROL(cltypeof<pulsar::math::BoxingQuat4f>(), new Quat4fPropertyControl);
    public:

        virtual bool OnDrawImGui(const string& name, Type* type, Object* prop) override;

    };
}