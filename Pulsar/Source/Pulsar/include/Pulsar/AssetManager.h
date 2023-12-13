#pragma once
#include <Pulsar/AssetObject.h>

namespace pulsar
{

    class AssetManager
    {
    public:
        virtual AssetObject_ref LoadAssetAtPath(string_view path) = 0;
        
        virtual AssetObject_ref LoadAssetById(ObjectHandle id) = 0;

        template<baseof_assetobject T>
        inline ObjectPtr<T> LoadAsset(string_view path, bool allowException = false)
        {
            auto ptr = LoadAssetAtPath(path).GetPtr();
            if (allowException && ptr == nullptr)
            {
               throw NullPointerException{};
            }
            return ObjectPtr<T>(object_cast<T>(ptr));
        }



        virtual ~AssetManager() = default;
    };

    AssetManager* GetAssetManager();

    static void TryFindOrLoadObject(ObjectHandle handle)
    {
        if (auto ptr = RuntimeObjectWrapper::GetObject(handle))
        {
        }
        else if(auto asset = GetAssetManager()->LoadAssetById(handle))
        {
        }
    }
    template <typename T>
    static void TryFindOrLoadObject(ObjectPtr<T>& obj)
    {
        if (auto ptr = RuntimeObjectWrapper::GetObject(obj.handle))
        {
            obj.Ptr = (T*)ptr;
        }
        else
        {
            if(auto asset = GetAssetManager()->LoadAssetById(obj.handle))
            {
                obj.Ptr = (T*)asset.Ptr;
            }
        }
    }

}