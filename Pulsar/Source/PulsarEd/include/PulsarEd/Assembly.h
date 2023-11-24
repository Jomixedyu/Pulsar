#pragma once

#include <Pulsar/Logger.h>
#include <Pulsar/Application.h>
#include "ForwardDeclare.h"


#ifdef _WIN32
    #ifdef PULSARED_BUILD_SHARED
        #ifdef PULSARED_EXPORT_API
            #define PULSARED_API __declspec(dllexport)
        #else
            #define PULSARED_API __declspec(dllimport)
        #endif
    #else
        #define PULSARED_API
    #endif
#else
    #define PULSARED_API
#endif


namespace pulsared
{
    CORELIB_DECL_ASSEMBLY(pulsared);

    using namespace ::pulsar;

    /*unsafe function*/
    inline class EditorAppInstance* EditorAppInst()
    {
        return reinterpret_cast<class EditorAppInstance*>(Application::inst());
    }
}
