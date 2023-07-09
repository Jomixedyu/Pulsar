#pragma once
#include "PropertyControl.h"
#include <Pulsar/EngineMath.h>

namespace pulsared
{
    class Vector3fPropertyControl : public PropertyControl
    {
        APATITEED_PROPERTY_CONTROL(cltypeof<pulsar::math::BoxingVector3f>(), new Vector3fPropertyControl);
    public:

        virtual void OnDrawImGui(const string& name, sptr<Object> prop) override;
        virtual Type* GetPropertyType() override
        {
            return cltypeof<pulsar::math::BoxingVector3f>();
        }
    };
}