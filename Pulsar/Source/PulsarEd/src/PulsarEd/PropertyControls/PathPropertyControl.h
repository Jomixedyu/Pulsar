#pragma once
#include "PropertyControl.h"
#include <CoreLib/Path.h>

namespace pulsared
{
    class PathPropertyControl : public PropertyControl
    {
        EDITOR_IMPL_PROPERTY_CONTROL(cltypeof<jxcorlib::Path>(), new PathPropertyControl);
    public:
        virtual bool OnDrawImGui(const string& name, Type* type, Object* prop) override;
    };
}
