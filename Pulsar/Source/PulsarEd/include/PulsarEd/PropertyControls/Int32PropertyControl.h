#pragma once
#include "PropertyControl.h"

namespace pulsared
{
    class Int32PropertyControl : public PropertyControl
    {
        EDITOR_IMPL_PROPERTY_CONTROL(cltypeof<Integer32>(), new Int32PropertyControl);
    public:

        virtual bool OnDrawImGui(const string& name, Type* type, Object* prop) override;

    };
}