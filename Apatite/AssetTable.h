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

    template<typename T>
    struct AssetReference
    {
        bool IsValid()
        {
            return AssetReferenceRegistry::IsValid(this->asset_guid);
        }
        sptr<T> GetAsset()
        {
            return sptr_cast<T>(AssetReferenceRegistry::FindAsset(this->asset_guid));
        }
        guid_t asset_guid;
    };
}