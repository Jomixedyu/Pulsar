#pragma once
#include <ApatiteEd/Assembly.h>
#include <Apatite/Subsystem.h>

namespace apatiteed
{
    class EditorSubsystem : public Subsystem
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::EditorSubsystem, Subsystem);
    public:
        virtual void OnEditorStartRender() {}
        virtual void OnEditorStopRender() {}
    };
}