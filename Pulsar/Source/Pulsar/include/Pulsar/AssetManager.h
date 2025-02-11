#pragma once
#include <Pulsar/AssetObject.h>

namespace pulsar
{

    class AssetManager
    {
    public:
        virtual RCPtr<AssetObject> LoadAssetAtPath(string_view path) = 0;
        
        virtual RCPtr<AssetObject> LoadAssetById(ObjectHandle id) = 0;

        template<baseof_assetobject T>
        inline RCPtr<T> LoadAsset(string_view path, bool allowException = false)
        {
            RCPtr<T> ptr = cref_cast<T>(LoadAssetAtPath(path));
            if (allowException && ptr == nullptr)
            {
               throw NullPointerException{};
            }
            return ptr;
        }



        virtual ~AssetManager() = default;
    };

    AssetManager* GetAssetManager();

    template <typename T>
    void TryLoadAssetRCPtr(RCPtr<T>& ptr)
    {
        if (ptr == nullptr)
        {
            ptr = GetAssetManager()->LoadAssetById(ptr.GetHandle());
        }
    }

}