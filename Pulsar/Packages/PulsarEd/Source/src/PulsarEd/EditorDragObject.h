#pragma once
#include "Assembly.h"

namespace pulsared
{
    struct EditorDragObject
    {
        enum ContentType
        {
            Object,
            FilePath,
        };

        char AssetPath[256];
        ObjectHandle Handle;
        ContentType Type;
    };

}