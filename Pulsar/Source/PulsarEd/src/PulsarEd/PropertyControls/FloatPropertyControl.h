#pragma once
#include "PropertyControl.h"

namespace pulsared
{
    class FloatPropertyControl : public PropertyControl
    {
        EDITOR_IMPL_PROPERTY_CONTROL(cltypeof<Single32>(), new FloatPropertyControl);
    public:

        virtual bool OnDrawImGui(const string& name, Type* type, Object* prop) override;

    };
}