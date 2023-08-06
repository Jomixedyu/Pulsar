#include "AssetPackage.h"
#include <unordered_set>

namespace pulsar
{
    static auto _Get()
    {
        static auto ptr = new hash_map<ObjectHandle, AssetPackageRef>();
        static struct Init
        {
            Init()
            {
                ptr->reserve(10000);
            }
        } _Init;
        return ptr;
    }

    static auto _GetAssetPackageSet()
    {
        static auto ptr = new std::unordered_set<AssetPackage*>;
        return ptr;
    }

    AssetPackageRef AssetPackageManager::GetAssetPackageInfo(const ObjectHandle& handle)
    {
        auto it = _Get()->find(handle);
        if (it != _Get()->end())
        {
            return it->second;
        }
        return {};
    }

    AssetPackageRef AssetPackageManager::LoadPackage(string_view path)
    {
        // load
        AssetPackage* package = nullptr;
        _GetAssetPackageSet()->insert(package);
        return AssetPackageRef();
    }

    bool AssetPackageManager::IsLoadedPackage(AssetPackage* package)
    {
        return _GetAssetPackageSet()->contains(package);
    }

    bool AssetPackageRef::IsLoaded() const
    {
        return AssetPackageManager::IsLoadedPackage(m_package);
    }

}
