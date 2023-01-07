#pragma once
#include "ObjectBase.h"
#include <CoreLib/Guid.h>
#include <Apatite/AssetObject.h>

namespace apatite
{

    class AssetReferenceRegistry
    {
    public:
        static bool IsLoadable(guid_t guid);
        static AssetObject_sp FindAsset(guid_t guid);
        static void Reset();
        static void RegisterAsset(guid_t guid, std::function<AssetObject_sp(guid_t)>&& getter);
    };

}