#pragma once
#include "Assembly.h"

namespace pulsared
{

    struct SceneObjectDragInfo
    {
        static constexpr string_view Name = "PULSARED_DRAG_SCENEOBJECT";
        Type* Type{};
        ObjectHandle Handle;
        guid_t SceneObjectGuid;
    };

    struct AssetObjectDragInfo
    {
        static constexpr string_view Name = "PULSARED_DRAG_ASSETOBJECT";
        Type* Type{};
        guid_t AssetGuid {};
        char Path[256] = {};
    };

} // namespace pulsared