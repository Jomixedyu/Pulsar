#pragma once

#ifdef _WIN32
    #ifdef JXCORLIBPLATFORM_SHARED
        #ifdef JXCORLIBPLATFORM_API
            #define JXCORLIBPLATFORM_API __declspec(dllexport)
        #else
            #define JXCORLIBPLATFORM_API __declspec(dllimport)
        #endif
    #else
        #define JXCORLIBPLATFORM_API
    #endif
#else
    #define JXCORLIBPLATFORM_API
#endif


