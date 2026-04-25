#pragma once

#include "ObjectBase.h"
#include <CoreLib/Events.hpp>
#include <stacktrace>
#include <fstream>
#include <mutex>

namespace pulsar
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

        // Initialize file logging. Writes to the specified path (append mode).
        // Each log entry is flushed immediately so crash logs are preserved.
        static void InitializeFileLogging(const string& logFilePath);

        // Shut down file logging and close the file stream.
        static void ShutdownFileLogging();

    private:
        static std::ofstream& GetLogFileStream();
        static std::mutex& GetLogFileMutex();
        static bool& GetFileLoggingEnabled();
    };

}
