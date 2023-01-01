#include "LogRecorder.h"

#include <Apatite/Logger.h>
#include <ctime>

namespace apatiteed
{
    static void _GetTime(string& str)
    {
        time_t t;
        std::time(&t);
        struct tm* p = std::localtime(&t);
        char buf[128];
        ::sprintf_s(buf, 128, "[%02d:%02d:%02d] ", p->tm_hour, p->tm_min, p->tm_sec);
        str = buf;
    }
    static void _Listener(LogLevel level, string_view log)
    {
        string info;
        info.reserve(log.size() + 36);
        _GetTime(info);
        switch (level)
        {
        case apatite::LogLevel::Info:
            info += "[Info] ";
            break;
        case apatite::LogLevel::Warning:
            info += "[Warning] ";
            break;
        case apatite::LogLevel::Error:
            info += "[Error] ";
            break;
        default:
            break;
        }
        info += log;
        LogRecorder::loglist.push_back(std::move(info));
    }

    array_list<string> LogRecorder::loglist;

    void LogRecorder::Initialize()
    {
        Logger::LogListener += _Listener;
    }

    void LogRecorder::Terminate()
    {
        Logger::LogListener -= _Listener;
    }
    void LogRecorder::Clear()
    {
        loglist.clear();
    }
}