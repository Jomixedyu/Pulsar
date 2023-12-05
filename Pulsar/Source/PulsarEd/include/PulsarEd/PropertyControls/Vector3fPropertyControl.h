#pragma once
#include "PropertyControl.h"
#include <Pulsar/EngineMath.h>

namespace pulsared
{
    class Vector3fPropertyControl : public PropertyControl
    {
        EDITOR_IMPL_PROPERTY_CONTROL(cltypeof<pulsar::math::BoxingVector3f>(), new Vector3fPropertyControl);
    public:

        virtual bool OnDrawImGui(const string& name, Type* type, Object* prop) override;

    };
}