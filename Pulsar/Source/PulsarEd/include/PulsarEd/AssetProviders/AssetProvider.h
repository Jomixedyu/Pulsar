#pragma once
#include <PulsarEd/Assembly.h>
#include <Pulsar/Assets/TextAsset.h>

#define PULSAR_DEFINE_ASSETPROVIDER(THISCLASS) \
static inline struct __assetprovider_t { \
__assetprovider_t() { ::pulsared::AssetProviderManager::Register(new THISCLASS); } \
} __assetprovider;


namespace pulsared
{
    class AssetProvider : public Object
    {
    public:
        virtual array_list<string> GetImportableExtNames() const = 0;
        virtual string GetExtNames() const = 0;
        virtual void RegisterIcon() = 0;
        virtual Type* GetAssetType() const = 0;
        virtual const char* GetProviderName() const = 0;
        virtual intptr_t GetProviderID() const = 0;
    };

    class AssetProviderManager final
    {
    public:
        static void Register(AssetProvider* provider);
        static void Cleanup();
        static AssetProvider* GetProvider(string_view ext);
        static array_list<AssetProvider*> GetProviders();
    };


    class TextAssetProvider : public AssetProvider
    {
        PULSAR_DEFINE_ASSETPROVIDER(TextAssetProvider);
    public:
        virtual void RegisterIcon() override {}
        virtual array_list<string> GetImportableExtNames() const override
        {
            return { ".txt", ".xml", ".json", ".toml" };
        }
        virtual string GetExtNames() const override
        {
            return ".pat" ;
        }
        virtual const char* GetProviderName() const override
        {
            return "TextAsset";
        }
        virtual Type* GetAssetType() const
        {
            return cltypeof<TextAsset>();
        }
        virtual intptr_t GetProviderID() const override
        {
            static bool b = 1;
            return reinterpret_cast<intptr_t>(&b);
        }
    };
}