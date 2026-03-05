#include "AssetProviders/AssetProvider.h"

namespace pulsared
{
    
    static auto& _GetProviders()
    {
         static array_list<AssetProvider*> providers;
         return providers;
    }

    void AssetProviderManager::Register(AssetProvider* provider)
    {
        _GetProviders().push_back(provider);
    }

    void AssetProviderManager::Cleanup()
    {
        _GetProviders().clear();
    }

    AssetProvider* AssetProviderManager::GetProvider(string_view ext)
    {
        for (auto provider : _GetProviders())
        {
            if (provider->GetExtNames() == ext)
            {
                return provider;
            }
        }
        return nullptr;
    }

    array_list<AssetProvider*> AssetProviderManager::GetProviders()
    {
        return _GetProviders();
    }
}