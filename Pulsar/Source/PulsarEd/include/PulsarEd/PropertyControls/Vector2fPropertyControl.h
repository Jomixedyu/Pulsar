#pragma once
#include "PropertyControl.h"
#include <Pulsar/EngineMath.h>

namespace pulsared
{
    class Vector2fPropertyControl : public PropertyControl
    {
        EDITOR_IMPL_PROPERTY_CONTROL(cltypeof<pulsar::math::BoxingVector2f>(), new Vector2fPropertyControl);
    public:

        virtual bool OnDrawImGui(const string& name, Object* prop) override;

    };
}