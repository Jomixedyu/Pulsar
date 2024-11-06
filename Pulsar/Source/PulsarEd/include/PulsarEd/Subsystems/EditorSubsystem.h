#pragma once
#include "Assembly.h"
#include <Pulsar/Subsystems/Subsystem.h>

namespace pulsared
{
    class EditorSubsystem : public Subsystem
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::EditorSubsystem, Subsystem);
    public:
        virtual void OnEditorStartRender() {}
        virtual void OnEditorStopRender() {}
    };
}