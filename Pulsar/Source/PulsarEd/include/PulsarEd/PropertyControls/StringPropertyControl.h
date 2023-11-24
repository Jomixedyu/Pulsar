#pragma once
#include "PropertyControl.h"

namespace pulsared
{
    class StringPropertyControl : public PropertyControl
    {
        EDITOR_IMPL_PROPERTY_CONTROL(cltypeof<String>(), new StringPropertyControl);
    public:

        virtual bool OnDrawImGui(const string& name, Object* prop) override;

    };
}