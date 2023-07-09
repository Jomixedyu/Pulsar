#pragma once
#include <Pulsar/EngineMath.h>
#include "Assembly.h"
#include "Workspace.h"

namespace pulsared
{

    class EditorUIConfig : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_PulsarEd, pulsared::EditorUIConfig, Object);

    public:
        CORELIB_REFL_DECL_FIELD(window_size);
        Vector2f window_size;
    };
    CORELIB_DECL_SHORTSPTR(EditorUIConfig);
}