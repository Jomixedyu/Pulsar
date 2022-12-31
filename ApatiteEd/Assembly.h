#pragma once

#include <Apatite/AssetObject.h>
#include <Apatite/Logger.h>
#include <Apatite/Application.h>

namespace apatiteed
{
    CORELIB_DECL_ASSEMBLY(ApatiteEd);

    using namespace ::apatite;


    /*unsafe function*/
    inline class EditorAppInstance* EditorAppInst()
    {
        return (class EditorAppInstance*)(Application::inst());
    }
}