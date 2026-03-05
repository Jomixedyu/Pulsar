#pragma once
#include "PropertyControl.h"

namespace pulsared
{
    class Matrix4fPropertyControl : public PropertyControl
    {
        EDITOR_IMPL_PROPERTY_CONTROL(cltypeof<pulsar::math::BoxingMatrix4f>(), new Matrix4fPropertyControl);
    public:

        virtual bool OnDrawImGui(const string& name, Type* type, Object* prop) override;

    };
}