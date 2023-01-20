#pragma once
#include "PropertyControl.h"
#include <Apatite/Math.h>

namespace apatiteed
{
    class Vector3fPropertyControl : public PropertyControl
    {
        APATITEED_PROPERTY_CONTROL(cltypeof<apatite::math::BoxingVector3f>(), new Vector3fPropertyControl);
    public:

        virtual void OnDrawImGui(const string& name, sptr<Object> prop) override;
        virtual Type* GetPropertyType() override
        {
            return cltypeof<apatite::math::BoxingVector3f>();
        }
    };
}