#include "AssetRegistry.h"

namespace apatite
{

    static auto table()
    {
        static auto table = new map<guid_t, AssetRegisterInfo>;
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

    bool AssetRegistry::IsLoadable(guid_t guid)
    {
        return _HasCache(guid) || _HasTable(guid);
    }

    AssetObject_sp AssetRegistry::FindAsset(guid_t guid)
    {
        return AssetObject_sp();
    }

    AssetObject_sp AssetRegistry::FindAssetAtPath(string_view path)
    {
        for (auto& item : *table())
        {
            if (item.second.path == path)
            {
                return item.second.getter(item.first);
            }
        }
        return nullptr;
    }
    
    void AssetRegistry::Reset()
    {
        table()->clear();
        cache()->clear();
    }
    void AssetRegistry::RegisterAsset(guid_t guid, AssetRegisterInfo info)
    {
        table()->emplace(guid, std::move(info));
    }
}