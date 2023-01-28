#pragma once
#include "PropertyControl.h"
#include <Apatite/Math.h>

namespace apatiteed
{
    class Quat4fPropertyControl : public PropertyControl
    {
        APATITEED_PROPERTY_CONTROL(cltypeof<apatite::math::BoxingQuat4f>(), new Quat4fPropertyControl);
    public:

        virtual void OnDrawImGui(const string& name, sptr<Object> prop) override;
        virtual Type* GetPropertyType() override
        {
            return cltypeof<apatite::math::BoxingQuat4f>();
        }
    };
}