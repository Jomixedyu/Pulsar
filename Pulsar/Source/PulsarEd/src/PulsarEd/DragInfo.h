#pragma once
#include "Assembly.h"

namespace pulsared
{

    struct ObjectPtrDragInfo
    {
        static constexpr string_view Name = "PULSARED_DRAG";
        Type* Type{};
        ObjectHandle ObjectHandle;
        string AssetPath;
    };

} // namespace pulsared