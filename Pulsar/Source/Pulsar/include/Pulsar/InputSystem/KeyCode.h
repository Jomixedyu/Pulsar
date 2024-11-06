#pragma once
#include <uinput/InputManager.h>

namespace pulsar
{
    using uinput::KeyCode;

    CORELIB_DEF_BOXING_ENUM_EXTERN(AssemblyObject_pulsar, pulsar, KeyCode, uinput::BuildKeyCodeMap);

}

CORELIB_DECL_BOXING(pulsar::KeyCode, pulsar::BoxingKeyCode);