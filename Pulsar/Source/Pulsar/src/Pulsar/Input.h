#pragma once
#include <uinput/InputKeyCode.h>
#include <set>

namespace pulsar
{
    using KeyCode = uinput::KeyCode;

    class Input
    {
    public:
        static void Initialize();
        static void Shutdown();
        static void Update(); // 每帧开始时调用，在 ProcessEvents 之前

        static bool GetKey(KeyCode key);        // 持续按住
        static bool GetKeyDown(KeyCode key);    // 本帧刚按下
        static bool GetKeyUp(KeyCode key);      // 本帧刚释放

    private:
        static int s_callbackHandle;
        static std::set<KeyCode> s_currentKeys;
        static std::set<KeyCode> s_previousKeys;
    };
}
