#pragma once
#include "PropertyControl.h"
#include <Pulsar/Math.h>

namespace pulsared
{
    class ColorPropertyControl : public PropertyControl
    {
        APATITEED_PROPERTY_CONTROL(cltypeof<jxcorlib::math::BoxingLinearColorf>(), new ColorPropertyControl);
    public:
        virtual void OnDrawImGui(const string& name, sptr<Object> prop) override;
        virtual Type* GetPropertyType() override
        {
            return cltypeof<jxcorlib::math::BoxingLinearColorf>();
        }
    };

}