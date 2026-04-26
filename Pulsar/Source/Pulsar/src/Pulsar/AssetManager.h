#pragma once
#include <Pulsar/AssetObject.h>

namespace pulsar
{

    class AssetManager
    {
    public:
        virtual RCPtr<AssetObject> LoadAssetAtPath(string_view path) = 0;
        
        virtual RCPtr<AssetObject> LoadAssetById(guid_t id) = 0;

        template<baseof_assetobject T>
        inline RCPtr<T> LoadAssetById(guid_t id, bool allowException = false)
        {
            RCPtr<T> ptr = cast<T>(LoadAssetById(id));
            if (allowException && ptr == nullptr)
            {
               throw NullPointerException{};
            }
            return ptr;
        }

        template<baseof_assetobject T>
        inline RCPtr<T> LoadAsset(string_view path, bool allowException = false)
        {
            RCPtr<T> ptr = cast<T>(LoadAssetAtPath(path));
            if (allowException && ptr == nullptr)
            {
               throw NullPointerException{};
            }
            return ptr;
        }

        virtual ~AssetManager() = default;

        static AssetManager* Get();
    };

}