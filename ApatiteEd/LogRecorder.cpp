#include "LogRecorder.h"

#include <Apatite/Logger.h>


namespace apatiteed
{
    static void _Listener(LogLevel level, string_view log)
    {
        string info;
        switch (level)
        {
        case apatite::LogLevel::Info:
            info = "[Info] ";
            break;
        case apatite::LogLevel::Warning:
            info = "[Warning] ";
            break;
        case apatite::LogLevel::Error:
            info = "[Error] ";
            break;
        default:
            break;
        }
        LogRecorder::loglist.push_back(info + string{ log });
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