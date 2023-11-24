#pragma once
#include <memory>
#include <chrono>
#include <string_view>

namespace jxcorlib
{
    struct StopWatch
    {
    public:
        void Start()
        {
            Latest = std::chrono::high_resolution_clock::now();
        }
        inline float Record()
        {
            auto now = std::chrono::high_resolution_clock::now();
            TimeCount = std::chrono::duration<float, std::chrono::milliseconds::period>(now - Latest).count();
            Latest = now;

            return TimeCount;
        }
        inline float Record(std::string_view info)
        {
            Record();
            if (IsPrintConsole)
            {
                printf("[%s] %s; %fms.\n", LogHead, info.data(), TimeCount);
            }
            return TimeCount;
        }
        StopWatch() : IsPrintConsole(false)
        {}
        StopWatch(std::string_view logHead) : IsPrintConsole(true)
        {
            memset(LogHead, 0, sizeof(LogHead));
            memcpy(LogHead, logHead.data(), logHead.length());
        }

        float TimeCount = 0;
        char LogHead[64]{};
        bool IsPrintConsole;
        std::chrono::steady_clock::time_point Latest;
    };
}