#pragma once

#include <Pulsar/AssetObject.h>
#include <Pulsar/Logger.h>
#include <Pulsar/Application.h>
#include <Pulsar/Node.h>


namespace pulsared
{
    CORELIB_DECL_ASSEMBLY(PulsarEd);

    using namespace ::pulsar;


    /*unsafe function*/
    inline class EditorAppInstance* EditorAppInst()
    {
        return (class EditorAppInstance*)(Application::inst());
    }
}