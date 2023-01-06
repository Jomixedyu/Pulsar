#pragma once

#include "ObjectBase.h"
#include <CoreLib/Events.hpp>
#include <stacktrace>

namespace apatite
{
    enum class LogLevel
    {
        Info,
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
        static void Log(LogLevel level, string_view str);

        static const char* GetLevelHead(LogLevel level);

        static inline Action<const LogRecord&> LogListener;
    };

}
