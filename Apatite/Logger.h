#pragma once

#include "ObjectBase.h"
#include <CoreLib/Events.hpp>

namespace apatite
{
    enum class LogLevel
    {
        Info,
        Warning,
        Error,
    };
    class Logger
    {
    public:
        static void Log(LogLevel level, string_view str);

        static inline Action<LogLevel, string_view> LogListener;
    };

}
