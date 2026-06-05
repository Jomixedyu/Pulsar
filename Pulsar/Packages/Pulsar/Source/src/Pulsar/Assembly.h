#pragma once

#include <CoreLib/Core.h>
#include <CoreLib/Reflection.h>

#ifdef _WIN32
    #if defined(PULSAR_BUILD_SHARED) && defined(PULSAR_EXPORT_API)
        #define PULSAR_API __declspec(dllexport)
    #elif defined(PULSARED_BUILD_SHARED) && !defined(PULSARED_EXPORT_API)
        #define PULSAR_API __declspec(dllimport)
    #else
        #define PULSAR_API
    #endif
#else
    #define PULSAR_API
#endif

namespace pulsar
{
    using namespace jxcorlib;
    CORELIB_DECL_ASSEMBLY(pulsar);
}

namespace pulsar
{
    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar,
        OSPlatform,
        Windows64
        );
}
CORELIB_DECL_BOXING(pulsar::OSPlatform, pulsar::BoxingOSPlatform);