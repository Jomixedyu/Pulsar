#pragma once
#include <PulsarEd/Assembly.h>

namespace pulsared
{
    class AssetImporterSettings
    {
    public:
        array_list<std::filesystem::path> ImportFiles;
        string TargetPath;
    };

    class AssetImporter
    {
    public:
        virtual ~AssetImporter() = default;
        virtual array_list<AssetObject_ref> Import(AssetImporterSettings* settings) = 0;
        virtual string GetImporterType() const = 0;
    };

    class AssetImporterFactory
    {
    public:
        virtual ~AssetImporterFactory() = default;
        virtual bool IsSupportFormat(string_view format) = 0;
        array_list<string> GetSupportFormats();
        virtual string GetDescription() = 0;
        virtual std::shared_ptr<AssetImporter> CreateImporter() = 0;
    };

    class AssetImporterManager
    {
    public:
        void RegisterFactory(string_view name, AssetImporterFactory* factory);
        void UnregisterFactory(string_view name);

    };
}
