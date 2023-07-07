#pragma once

#include <Pulsar/AssetObject.h>
#include <Pulsar/Logger.h>
#include <Pulsar/Application.h>
#include <PulsarEd/Assembly.h>

namespace pulsarged
{
    CORELIB_DECL_ASSEMBLY(PulsarGEd);

    using namespace ::pulsar;
    using namespace ::pulsared;


    /*unsafe function*/
    inline class EditorAppInstance* EditorAppInst()
    {
        return (class EditorAppInstance*)(Application::inst());
    }
}