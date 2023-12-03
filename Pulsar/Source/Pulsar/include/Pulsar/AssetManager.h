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
        inline ObjectPtr<T> LoadAsset(string_view path)
        {
            auto ptr = LoadAssetAtPath(path).GetPtr();
            return ObjectPtr<T>(object_cast<T>(ptr));
        }



        virtual ~AssetManager() { }
    };

    AssetManager* GetAssetManager();

}