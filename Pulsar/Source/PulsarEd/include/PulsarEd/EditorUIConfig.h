#pragma once
#include <Pulsar/EngineMath.h>
#include "Assembly.h"
#include "Workspace.h"

namespace pulsared
{

    class EditorUIConfig : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::EditorUIConfig, Object);

    public:
        CORELIB_REFL_DECL_FIELD(WindowSize);
        Vector2f WindowSize;
    };
    CORELIB_DECL_SHORTSPTR(EditorUIConfig);
}