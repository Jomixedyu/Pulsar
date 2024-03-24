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

            std::u8string u8str = (char8_t*)file.c_str();
            auto fileBytes = FileUtil::ReadAllBytes(u8str);

            int32_t width{0}, height{0}, channel{0};
            gfx::LoadImageFromMemory(fileBytes.data(), fileBytes.size(), &width, &height, &channel, 0, true);
            asset->FromNativeData(fileBytes.data(), fileBytes.size(), true, width, height, channel);

            asset->SetIsSRGB(true);

            auto assetPath = settings->TargetPath + "/" + PathUtil::GetFilenameWithoutExt(file);
            assetPath = AssetDatabase::GetUniquePath(assetPath);
            AssetDatabase::CreateAsset(asset.get(), assetPath);
            importAssets.push_back(asset.get());
        }

        return importAssets;
    }
} // namespace pulsared