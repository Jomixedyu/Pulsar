#pragma once
#include "ObjectBase.h"
#include <CoreLib/Guid.h>
#include <Apatite/AssetObject.h>

namespace apatite
{
    class AssetTable
    {
    public:
        static AssetObject_sp FindAsset(guid_t);
        static void FindAssetAsync(guid_t, const std::function<void(AssetObject_sp)>& callback);
    };
}