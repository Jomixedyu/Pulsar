#include "EditorLogRecorder.h"
#include <ctime>

namespace pulsared
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
    static void _Listener(const LogRecord& record)
    {
        EditorLogRecord rec;
        rec.level = record.level;

        rec.record_info = record.GetFriendlyInfo();

        auto st = std::to_string(record.stacktrace);
        rec.stacktrace_info.reserve(32 + rec.record_info.size() + st.size());

        rec.stacktrace_info.append(rec.record_info);
        rec.stacktrace_info.append("\n\nstacktrace:\n");
        rec.stacktrace_info.append(std::move(st));

        std::lock_guard<std::mutex> lock(EditorLogRecorder::GetMutex());
        EditorLogRecorder::GetLogList().push_back(std::move(rec));
    }

    array_list<EditorLogRecord>& EditorLogRecorder::GetLogList()
    {
        static array_list<EditorLogRecord> loglist;
        return loglist;
    }

    std::mutex& EditorLogRecorder::GetMutex()
    {
        static std::mutex mtx;
        return mtx;
    }

    void EditorLogRecorder::Initialize()
    {
        Logger::LogListener += _Listener;
    }

    void EditorLogRecorder::Terminate()
    {
        Logger::LogListener -= _Listener;
    }
    void EditorLogRecorder::Clear()
    {
        std::lock_guard<std::mutex> lock(GetMutex());
        GetLogList().clear();
    }
}