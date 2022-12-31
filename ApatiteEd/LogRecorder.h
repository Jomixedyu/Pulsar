#pragma once
#include <ApatiteEd/Assembly.h>


namespace apatiteed
{

    class LogRecorder
    {
    public:
        static void Initialize();
        static void Terminate();
        static void Clear();
        static array_list<string> loglist;
    };
}