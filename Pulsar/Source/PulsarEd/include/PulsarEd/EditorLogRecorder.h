#pragma once
#include <PulsarEd/Assembly.h>
#include <Pulsar/Logger.h>

namespace pulsared
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