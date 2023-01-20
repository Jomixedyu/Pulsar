#pragma once

#include "ObjectBase.h"
#include <CoreLib/Events.hpp>
#include <stacktrace>

namespace apatite
{
    enum class LogLevel
    {
        Info = 1,
        Warning,
        Error,
    };

    struct LogRecord
    {
        string text;
        LogLevel level;
        std::stacktrace stacktrace;
        string time;

        string GetFriendlyInfo() const;
    };

    class Logger
    {
    public:
        static void Log(string_view str, LogLevel level = LogLevel::Info);

        static const char* GetLevelHead(LogLevel level);

        static inline Action<const LogRecord&> LogListener;
    };

}
