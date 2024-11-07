#pragma once
#include "Assembly.h"
#include "Pulsar/EngineMath.h"

namespace pulsar
{
    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar,
                     InputValueType,
                     Bool,
                     Float,
                     Axis2D,
                     Axis3D);
}
CORELIB_DECL_BOXING(pulsar::InputValueType, pulsar::BoxingInputValueType);

namespace pulsar
{
    struct InputActionValue
    {
        InputValueType m_type{};
        Vector3f m_value{};
    };
}