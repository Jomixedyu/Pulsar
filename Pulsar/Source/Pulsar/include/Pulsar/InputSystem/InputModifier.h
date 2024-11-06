#pragma once
#include "Assembly.h"
#include "InputValueType.h"

namespace pulsar
{


    class InputModifier : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputModifier, Object);
    public:
        virtual InputActionValue Modify(const InputActionValue& value, float dt) { return value; }

    };
    CORELIB_DECL_SHORTSPTR(InputModifier);

    class InputModifierNegate : public InputModifier
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputModifierNegate, InputModifier);
    public:
        InputActionValue Modify(const InputActionValue& value, float dt) override
        {
            InputActionValue ret = value;
            ret.m_value = value.m_value * Vector3f(m_x ? -1 : 1, m_y ? -1 : 1, m_z ? -1 : 1);
            return ret;
        }

        CORELIB_REFL_DECL_FIELD(m_x);
        bool m_x = true;
        CORELIB_REFL_DECL_FIELD(m_y);
        bool m_y = true;
        CORELIB_REFL_DECL_FIELD(m_z);
        bool m_z = true;
    };
    CORELIB_DECL_SHORTSPTR(InputModifierNegate);
}
