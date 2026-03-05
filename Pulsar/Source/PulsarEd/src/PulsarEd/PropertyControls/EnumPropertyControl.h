#pragma once
#include "PropertyControl.h"

namespace pulsared
{
    class EnumPropertyControl : public PropertyControl
    {
        EDITOR_IMPL_PROPERTY_CONTROL(cltypeof<Enum>(), new EnumPropertyControl);
    public:

        virtual bool OnDrawImGui(const string& name, Type* type, Object* prop) override;

    };
}