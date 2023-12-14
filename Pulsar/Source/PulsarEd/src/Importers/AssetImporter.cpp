#include "Importers/AssetImporter.h"

namespace pulsared
{

    bool AssetImporterFactory::IsSupportFormat(string_view format) const
    {
        return std::ranges::find(m_supportedFormats, format) != m_supportedFormats.end();
    }

    string AssetImporterFactory::GetFilter() const
    {
        return std::views::all(m_supportedFormats) | std::views::transform([](auto& v) { return string{"*"} + v; }) | std::views::join_with(string{"|"}) | std::ranges::to<string>();
    }
    sptr<AssetImporterSettings> AssetImporterFactory::CreateImporterSettings()
    {
        return mksptr(new AssetImporterSettings);
    }

    AssetImporterFactory* AssetImporterFactoryManager::FindFactoryByExt(string_view ext)
    {
        for (const auto& factory : factories | std::views::values)
        {
            if(factory->IsSupportFormat(ext))
            {
                return factory.get();
            }
        }
        return nullptr;
    }

    AssetImporterFactory* AssetImporterFactoryManager::GetFactory(string_view name)
    {
        for (auto& [factoryName, factory] : factories)
        {
            if (factoryName == name)
                return factory.get();
        }
        return nullptr;
    }

    array_list<AssetImporterFactory*> AssetImporterFactoryManager::GetFactories()
    {
        return std::views::all(factories) | std::views::values | std::views::transform([](const std::unique_ptr<AssetImporterFactory>& v) { return v.get(); }) | std::ranges::to<array_list<AssetImporterFactory*>>();
    }
} // namespace pulsared