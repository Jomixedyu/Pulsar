#pragma once
#include "PropertyControl.h"
#include <CoreLib/Guid.h>

namespace pulsared
{
    class GuidPropertyControl : public PropertyControl
    {
        EDITOR_IMPL_PROPERTY_CONTROL(cltypeof<jxcorlib::Guid>(), new GuidPropertyControl);
    public:

        virtual bool OnDrawImGui(const string& name, Type* type, Object* prop, std::span<Attribute*> attrs = {}) override;

    };
}