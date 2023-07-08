#pragma once
#include "PropertyControl.h"

namespace pulsared
{
    class Matrix4fPropertyControl : public PropertyControl
    {
        APATITEED_PROPERTY_CONTROL(cltypeof<pulsar::math::BoxingMatrix4f>(), new Matrix4fPropertyControl);
    public:

        virtual void OnDrawImGui(const string& name, sptr<Object> prop) override;
        virtual Type* GetPropertyType() override
        {
            return cltypeof<pulsar::math::BoxingMatrix4f>();
        }
    };
}