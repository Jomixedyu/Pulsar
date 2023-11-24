#pragma once
#include "AssetObject.h"


namespace pulsar
{
    class AssetPackage
    {
        AssetPackage()
        {
        }
        AssetObject_ref LoadAsset(const ObjectHandle& handle)
        {

        }
    private:
        struct AssetInfo
        {
            string name;
            string path;
            string type;
        };

        string m_packageName;
        hash_map<ObjectHandle, AssetInfo> m_assetInfos;
    };

    struct AssetPackageRef final
    {
    public:
        AssetPackageRef() {}
        AssetPackageRef(AssetPackage*)
        {

        }
        AssetPackage* Get() const
        {
            return m_package;
        }
        bool IsLoaded() const;
        index_string GetName() const { return m_name; }
    protected:
        index_string m_name;
        AssetPackage* m_package = nullptr;
    };

    class AssetPackageManager
    {
    public:
        static AssetPackageRef GetAssetPackageInfo(const ObjectHandle& handle);
        static AssetPackageRef LoadPackage(string_view path);
        static bool IsLoadedPackage(AssetPackage* package);
    };
}