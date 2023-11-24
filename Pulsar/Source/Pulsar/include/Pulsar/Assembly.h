#pragma once

#ifdef _WIN32
    #ifdef PULSAR_BUILD_SHARED && PULSAR_EXPORT_API
        #define PULSAR_API __declspec(dllexport)
    #elif PULSARED_BUILD_SHARED && !PULSARED_EXPORT_API
        #define PULSAR_API __declspec(dllimport)
    #else
        #define PULSAR_API
    #endif
#else
    #define PULSAR_API
#endif

