#pragma once
#include "PropertyControl.h"
#include <Apatite/Math.h>

namespace apatiteed
{
    class MaterialPropertyControl : public PropertyControl
    {
        APATITEED_PROPERTY_CONTROL(cltypeof<Material>(), new MaterialPropertyControl);
    public:

        virtual void OnDrawImGui(const string& name, sptr<Object> prop) override;
        virtual Type* GetPropertyType() override
        {
            return cltypeof<Material>();
        }
    };
}