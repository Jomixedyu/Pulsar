#include "AssetTable.h"

namespace apatite
{

    static auto table()
    {
        static auto table = new map<guid_t, std::function<AssetObject_sp(guid_t)>>;
        return table;
    }

    static auto cache()
    {
        static auto cache = new map<guid_t, AssetObject_wp>;
        return cache;
    }

    static bool _HasCache(guid_t guid)
    {
        auto it = cache()->find(guid);
        if (it != cache()->end())
        {
            return !it->second.expired();
        }
        return false;
    }
    static bool _HasTable(guid_t guid)
    {
        auto it = table()->find(guid);
        return it != table()->end();
    }

    bool AssetReferenceRegistry::IsLoadable(guid_t guid)
    {
        return _HasCache(guid) || _HasTable(guid);
    }

    AssetObject_sp AssetReferenceRegistry::FindAsset(guid_t guid)
    {
        return AssetObject_sp();
    }
    
    void AssetReferenceRegistry::Reset()
    {
        table()->clear();
        cache()->clear();
    }
    void AssetReferenceRegistry::RegisterAsset(guid_t guid, std::function<AssetObject_sp(guid_t)>&& getter)
    {
        table()->emplace(guid, std::move(getter));
    }
}