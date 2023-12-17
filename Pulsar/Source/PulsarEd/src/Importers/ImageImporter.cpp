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

            auto fileBytes = FileUtil::ReadAllBytes(file);
            asset->LoadHostResource(fileBytes.data(), fileBytes.size());


            auto assetRef = AssetObject_ref(asset.get());
            auto assetPath = settings->TargetPath + "/" + PathUtil::GetFilenameWithoutExt(file);
            assetPath = AssetDatabase::GetUniquePath(assetPath);
            AssetDatabase::CreateAsset(assetRef, assetPath);
            importAssets.push_back(assetRef);
        }

        return importAssets;
    }
} // namespace pulsared