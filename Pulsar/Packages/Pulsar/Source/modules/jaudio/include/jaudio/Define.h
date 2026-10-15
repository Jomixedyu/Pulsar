#pragma once

#ifdef JAUDIO_BUILD_SHARED
    #ifdef _WIN32
        #ifdef JAUDIO_EXPORT_API
            #define JAUDIO_API __declspec(dllexport)
        #else
            #define JAUDIO_API __declspec(dllimport)
        #endif
    #else
        #define JAUDIO_API
    #endif
#else
    #define JAUDIO_API
#endif

