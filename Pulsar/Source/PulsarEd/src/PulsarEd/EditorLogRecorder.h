#pragma once
#include <PulsarEd/Assembly.h>
#include <Pulsar/Logger.h>
#include <mutex>

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
        // 调用前必须持有 GetMutex() 的锁
        static array_list<EditorLogRecord>& GetLogList();
        static std::mutex& GetMutex();
    };
}
