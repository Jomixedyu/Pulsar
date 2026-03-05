#pragma once
#include "PropertyControl.h"
#include <Pulsar/EngineMath.h>
#include <Pulsar/ObjectBase.h>

namespace pulsared
{
    class ObjectPropertyControl : public PropertyControl
    {
        EDITOR_IMPL_PROPERTY_CONTROL(cltypeof<ObjectBase>(), new ObjectPropertyControl);
    public:

        virtual bool OnDrawImGui(const string& name, Type* type, Object* prop) override;

    };
}