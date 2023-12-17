#pragma once
#include <ranges>
#include <PulsarEd/Assembly.h>

#define PULSARED_ASSET_IMPORTER_FACTORY(NAME) static inline struct __assetfactory_##NAME{ \
    __assetfactory_##NAME() { ::pulsared::AssetImporterFactoryManager::RegisterFactory(#NAME, std::make_unique<NAME>()); } } \
__assetfactory_##NAME##_;

namespace pulsared
{
    class PULSARED_API AssetImporterSettings : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, AssetImporterSettings, Object);
    public:
        CORELIB_REFL_DECL_FIELD(ImportFiles);
        List_sp<string> ImportFiles = mksptr(new List<string>);

        CORELIB_REFL_DECL_FIELD(TargetPath);
        string TargetPath;
    };

    class PULSARED_API AssetImporter
    {
    public:
        virtual ~AssetImporter() = default;
        virtual array_list<AssetObject_ref> Import(AssetImporterSettings* settings) = 0;
        virtual string GetImporterType() const = 0;
    };

    class PULSARED_API AssetImporterFactory
    {
    public:
        virtual ~AssetImporterFactory() = default;
        virtual bool IsSupportFormat(string_view format) const;
        const array_list<string>& GetSupportedFormats() const { return m_supportedFormats; }
        virtual string GetFilter() const;

        virtual string_view GetDescription() const = 0;
        virtual std::shared_ptr<AssetImporter> CreateImporter() = 0;
        virtual sptr<AssetImporterSettings> CreateImporterSettings();
    protected:
        array_list<string> m_supportedFormats;
    };

    class PULSARED_API AssetImporterFactoryManager
    {
    public:
        static void RegisterFactory(string_view name, std::unique_ptr<AssetImporterFactory>&& factory)
        {
            factories[name] = std::move(factory);
        }
        static void UnregisterFactory(string_view name)
        {
            factories.erase(name);
        }
        static AssetImporterFactory* FindFactoryByExt(string_view ext);
        static AssetImporterFactory* GetFactory(string_view name);
        static array_list<AssetImporterFactory*> GetFactories();

    protected:
        static inline hash_map<string_view, std::unique_ptr<AssetImporterFactory>> factories;
    };
}
