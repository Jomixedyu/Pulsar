#include "Importers/ImageImporter.h"

#include "AssetDatabase.h"

#include <Pulsar/Assets/Texture2D.h>
#include <gfx/GFXImage.h>
namespace pulsared
{

    array_list<AssetObject_ref> ImageImporter::Import(AssetImporterSettings* settings)
    {
        array_list<AssetObject_ref> importAssets;
        for (auto& file : *settings->ImportFiles)
        {
            auto asset = mksptr(new Texture2D);
            asset->Construct();

            std::u8string u8str = (char8_t*)file.c_str();
            auto fileBytes = FileUtil::ReadAllBytes(u8str);

            asset->LoadHostResource(fileBytes.data(), fileBytes.size());

            asset->SetIsSRGB(true);

            auto assetRef = AssetObject_ref(asset.get());
            auto assetPath = settings->TargetPath + "/" + PathUtil::GetFilenameWithoutExt(file);
            assetPath = AssetDatabase::GetUniquePath(assetPath);
            AssetDatabase::CreateAsset(assetRef, assetPath);
            importAssets.push_back(assetRef);
        }

        return importAssets;
    }
} // namespace pulsared