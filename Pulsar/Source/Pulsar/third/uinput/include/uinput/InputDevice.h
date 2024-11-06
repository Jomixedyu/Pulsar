#pragma once

namespace uinput
{
    class InputDevice
    {
    public:
        void SetActive(bool b);
    };

    class InputDeviceKeyboard : public InputDevice
    {
    public:
    };

    class InputDeviceMouse : public InputDevice
    {
    public:
    };
}