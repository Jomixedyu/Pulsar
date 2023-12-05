#pragma once
#include "PropertyControl.h"

namespace pulsared
{
    class BoolPropertyControl : public PropertyControl
    {
        EDITOR_IMPL_PROPERTY_CONTROL(cltypeof<Boolean>(), new BoolPropertyControl);
    public:

        virtual bool OnDrawImGui(const string& name, Type* type, Object* prop) override;

    };
}