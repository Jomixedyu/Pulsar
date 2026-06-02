#include <Pulsar/Logger.h>
#include <iostream>
#include <ctime>
#include <filesystem>

namespace pulsar
{
    static void _GetTime(string& str)
    {
        time_t t;
        std::time(&t);
        struct tm* p = std::localtime(&t);
        char buf[128];
        ::sprintf_s(buf, 128, "%02d:%02d:%02d", p->tm_hour, p->tm_min, p->tm_sec);
        str = buf;
    }

    string LogRecord::GetFriendlyInfo() const
    {
        string ret;
        ret.reserve(16 + this->time.size() + this->text.size());
        ret.append(Logger::GetLevelHead(this->level));
        ret.append("[");
        ret.append(this->time);
        ret.append("]");
        ret.append(this->text);
        return ret;
    }

    using namespace std;

    // ---- file logging internals ----
    static std::ofstream g_logFileStream;
    static std::mutex g_logFileMutex;
    static bool g_fileLoggingEnabled = false;

    std::ofstream& Logger::GetLogFileStream()
    {
        return g_logFileStream;
    }

    std::mutex& Logger::GetLogFileMutex()
    {
        return g_logFileMutex;
    }

    bool& Logger::GetFileLoggingEnabled()
    {
        return g_fileLoggingEnabled;
    }

    void Logger::InitializeFileLogging(const string& logFilePath)
    {
        std::lock_guard lock(g_logFileMutex);
        if (g_fileLoggingEnabled)
        {
            g_logFileStream.close();
        }

        // Ensure parent directory exists
        std::filesystem::path p(logFilePath);
        if (!p.parent_path().empty() && !std::filesystem::exists(p.parent_path()))
        {
            std::filesystem::create_directories(p.parent_path());
        }

        g_logFileStream.open(logFilePath, std::ios::out | std::ios::app | std::ios::binary);
        if (g_logFileStream.is_open())
        {
            g_fileLoggingEnabled = true;

            // Write session header
            time_t now = std::time(nullptr);
            struct tm* local = std::localtime(&now);
            char header[128];
            ::sprintf_s(header, 128, "\n===== Log session started at %04d-%02d-%02d %02d:%02d:%02d =====\n",
                        local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
                        local->tm_hour, local->tm_min, local->tm_sec);
            g_logFileStream.write(header, strlen(header));
            g_logFileStream.flush();
        }
    }

    void Logger::ShutdownFileLogging()
    {
        std::lock_guard lock(g_logFileMutex);
        if (g_fileLoggingEnabled && g_logFileStream.is_open())
        {
            time_t now = std::time(nullptr);
            struct tm* local = std::localtime(&now);
            char footer[128];
            ::sprintf_s(footer, 128, "===== Log session ended at %04d-%02d-%02d %02d:%02d:%02d =====\n\n",
                        local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
                        local->tm_hour, local->tm_min, local->tm_sec);
            g_logFileStream.write(footer, strlen(footer));
            g_logFileStream.flush();
            g_logFileStream.close();
        }
        g_fileLoggingEnabled = false;
    }

    void Logger::Log(string_view str, LogLevel level)
    {
        LogRecord record;
        record.level = level;
        if (level != LogLevel::Info)
        {
            record.stacktrace = std::stacktrace::current(1);
        }
        _GetTime(record.time);
        record.text.reserve(str.size() + 16);

        record.text.append(str);

        cout << GetLevelHead(level) << record.text << endl;

        // Write to file immediately (flush on every line for crash resilience)
        if (g_fileLoggingEnabled)
        {
            std::lock_guard lock(g_logFileMutex);
            if (g_logFileStream.is_open())
            {
                g_logFileStream << GetLevelHead(level)
                                << "[" << record.time << "]"
                                << record.text << '\n';
                g_logFileStream.flush();
            }
        }

        LogListener.Invoke(record);
    }

    const char* Logger::GetLevelHead(LogLevel level)
    {
        switch (level)
        {
        case pulsar::LogLevel::Info:
            return "[Info]";
        case pulsar::LogLevel::Warning:
            return"[Warning]";
        case pulsar::LogLevel::Error:
            return "[Error]";
        }
        return nullptr;
    }

}