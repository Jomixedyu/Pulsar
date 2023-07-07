#pragma once
#include "PropertyControl.h"

namespace pulsared
{
    class FloatPropertyControl : public PropertyControl
    {
        APATITEED_PROPERTY_CONTROL(cltypeof<Single32>(), new FloatPropertyControl);
    public:

        virtual void OnDrawImGui(const string& name, sptr<Object> prop) override;
        virtual Type* GetPropertyType() override
        {
            return cltypeof<Single32>();
        }
    };
}