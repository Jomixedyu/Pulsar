#pragma once
#include <Apatite/Math.h>
#include "Assembly.h"
#include "Project.h"

namespace apatiteed
{

    class EditorUIConfig : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_ApatiteEd, apatiteed::EditorUIConfig, Object);

    public:
        CORELIB_REFL_DECL_FIELD(window_size);
        Vector2f window_size;
    };
    CORELIB_DECL_SHORTSPTR(EditorUIConfig);
}