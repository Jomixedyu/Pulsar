#pragma once
#include <cstdint>
#include <string>
#include <string_view>

namespace pulsar::detail
{
    namespace SystemInterface
    {
        bool InitializeWindow(std::string_view title, int width, int height);
        void* GetWindow();
        void TerminateWindow();

        void GetResolution(int* out_width, int* out_height);
        void SetResolution(int width, int height);


        void PollEvents();

        void RequestQuitEvents();
        bool GetIsQuit();
        void SetRequestQuitCallBack(bool(*funptr)());
        void SetQuitCallBack(void(*funcptr)());


    }
}