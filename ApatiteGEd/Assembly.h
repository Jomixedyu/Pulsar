#pragma once

#include <Apatite/AssetObject.h>
#include <Apatite/Logger.h>
#include <Apatite/Application.h>
#include <ApatiteEd/Assembly.h>

namespace apatiteged
{
    CORELIB_DECL_ASSEMBLY(ApatiteGEd);

    using namespace ::apatite;
    using namespace ::apatiteed;


    /*unsafe function*/
    inline class EditorAppInstance* EditorAppInst()
    {
        return (class EditorAppInstance*)(Application::inst());
    }
}