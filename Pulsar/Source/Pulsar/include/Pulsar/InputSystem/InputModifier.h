#pragma once
#include "Assembly.h"
#include "InputValueType.h"

namespace pulsar
{
    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar, InputModifierSizzleMode,
        XYZ,
        YXZ,
        ZXY);
}
CORELIB_DECL_BOXING(pulsar::InputModifierSizzleMode, pulsar::BoxingInputModifierSizzleMode);

namespace pulsar
{


    class InputModifier : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputModifier, Object);
    public:
        virtual InputActionValue Modify(const InputActionValue& value, float dt) { return value; }

        CORELIB_REFL_DECL_FIELD(m_enabled);
        bool m_enabled = false;
    };
    CORELIB_DECL_SHORTSPTR(InputModifier);

    class InputModifierNegate : public InputModifier
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputModifierNegate, InputModifier);
    public:
        InputActionValue Modify(const InputActionValue& value, float dt) override
        {
            InputActionValue ret = value;
            ret.m_value = value.m_value * Vector3f(m_x ? -1.f : 1.f, m_y ? -1.f : 1.f, m_z ? -1.f : 1.f);
            return ret;
        }

        CORELIB_REFL_DECL_FIELD(m_x);
        bool m_x = false;
        CORELIB_REFL_DECL_FIELD(m_y);
        bool m_y = false;
        CORELIB_REFL_DECL_FIELD(m_z);
        bool m_z = false;
    };
    CORELIB_DECL_SHORTSPTR(InputModifierNegate);

    class InputModifierSwizzle : public InputModifier
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputModifierSwizzle, InputModifier);
    public:

        InputActionValue Modify(const InputActionValue& value, float dt) override
        {
            InputActionValue ret = value;
            switch (m_axis)
            {
            case InputModifierSizzleMode::XYZ: {
                break;
            }
            case InputModifierSizzleMode::YXZ: {
                ret.m_value.x = value.m_value.y;
                ret.m_value.y = value.m_value.x;
                break;
            }
            case InputModifierSizzleMode::ZXY:
                ret.m_value.x = value.m_value.z;
                ret.m_value.y = value.m_value.x;
                ret.m_value.z = value.m_value.y;
                break;
            }
            return ret;
        }

        CORELIB_REFL_DECL_FIELD(m_axis);
        InputModifierSizzleMode m_axis{};
    };
    CORELIB_DECL_SHORTSPTR(InputModifierSwizzle);

    class InputModifierSmooth : public InputModifier
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputModifierSmooth, InputModifier);
    public:
        InputActionValue Modify(const InputActionValue& value, float dt) override
        {
            InputActionValue ret = value;
            ret.m_value = value.m_value * Vector3f(m_x ? -1.f : 1.f, m_y ? -1.f : 1.f, m_z ? -1.f : 1.f);
            return ret;
        }

        CORELIB_REFL_DECL_FIELD(m_x);
        bool m_x = false;
        CORELIB_REFL_DECL_FIELD(m_y);
        bool m_y = false;
        CORELIB_REFL_DECL_FIELD(m_z);
        bool m_z = false;
    };
    CORELIB_DECL_SHORTSPTR(InputModifierSmooth);
}
