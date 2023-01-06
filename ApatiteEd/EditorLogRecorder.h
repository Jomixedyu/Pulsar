#pragma once
#include <ApatiteEd/Assembly.h>
#include <Apatite/Logger.h>

namespace apatiteed
{
    struct EditorLogRecord
    {
        LogLevel level;
        string record_info;
        string stacktrace_info;
    };

    class EditorLogRecorder
    {
    public:
        static void Initialize();
        static void Terminate();
        static void Clear();
        static array_list<EditorLogRecord> loglist;
    };
}