#include "Importers/ImageImporter.h"

#include "AssetDatabase.h"

#include <Pulsar/Assets/Texture2D.h>
#include <gfx/GFXImage.h>
namespace pulsared
{

    array_list<RCPtr<AssetObject>> ImageImporter::Import(AssetImporterSettings* settings)
    {
        array_list<RCPtr<AssetObject>> importAssets;
        for (auto& file : *settings->ImportFiles)
        {
            auto asset = mksptr(new Texture2D);
            asset->Construct();

            std::filesystem::path filePath { (char8_t*)file.c_str() };

            auto fileBytes = FileUtil::ReadAllBytes(filePath);

            int32_t width{0}, height{0}, channel{0};

            gfx::LoadImageInfo(fileBytes.data(), fileBytes.size(), &width, &height, &channel);

            asset->FromNativeData(fileBytes.data(), fileBytes.size(), true, width, height, channel);

            auto assetPath = settings->TargetPath + "/" + PathUtil::GetFilenameWithoutExt(file);
            assetPath = AssetDatabase::GetUniquePath(assetPath);
            AssetDatabase::CreateAsset(asset.get(), assetPath);
            importAssets.push_back(asset.get());
        }

        return importAssets;
    }
} // namespace pulsared