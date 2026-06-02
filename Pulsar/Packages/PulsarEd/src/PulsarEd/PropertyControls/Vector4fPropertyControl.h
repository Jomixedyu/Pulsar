#pragma once
#include "PropertyControl.h"
#include <Pulsar/EngineMath.h>

namespace pulsared
{
    class Vector4fPropertyControl : public PropertyControl
    {
        EDITOR_IMPL_PROPERTY_CONTROL(cltypeof<pulsar::math::BoxingVector4f>(), new Vector4fPropertyControl);
    public:

        virtual bool OnDrawImGui(const string& name, Type* type, Object* prop, std::span<Attribute*> attrs = {}) override;

    };
}
