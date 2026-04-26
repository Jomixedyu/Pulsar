#include "Importers/ImageImporter.h"

#include "AssetDatabase.h"

#include <Pulsar/Assets/Texture2D.h>
#include <gfx/GFXImage.h>
#include <algorithm>
namespace pulsared
{

    static bool IsNormalMapByFileName(const std::string& name)
    {
        auto lower = name;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower.find("_normal") != std::string::npos || lower.find("_nrm") != std::string::npos;
    }

    static bool IsDataTextureByFileName(const std::string& name)
    {
        auto lower = name;
        std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);
        return lower.find("_metallic") != std::string::npos ||
               lower.find("_roughness") != std::string::npos ||
               lower.find("_ao") != std::string::npos ||
               lower.find("_mask") != std::string::npos ||
               lower.find("_opacity") != std::string::npos ||
               lower.find("_height") != std::string::npos ||
               lower.find("_specular") != std::string::npos;
    }

    array_list<RCPtr<AssetObject>> ImageImporter::Import(AssetImporterSettings* settings)
    {
        array_list<RCPtr<AssetObject>> importAssets;
        for (auto& file : *settings->ImportFiles)
        {
            auto asset = NewAssetObject<Texture2D>();

            std::filesystem::path filePath = file;

            auto fileBytes = FileUtil::ReadAllBytes(filePath);

            int32_t width{0}, height{0}, channel{0};

            gfx::LoadImageInfo(fileBytes.data(), fileBytes.size(), &width, &height, &channel);

            asset->FromNativeData(fileBytes.data(), fileBytes.size(), true, width, height, channel);

            auto fileName = PathUtil::GetFilenameWithoutExt(file.filename().string());
            if (IsNormalMapByFileName(fileName))
            {
                asset->SetCompressedFormat(TextureCompressionFormat::NormalMap_Compressed);
            }
            else if (IsDataTextureByFileName(fileName))
            {
                asset->SetCompressedFormat(TextureCompressionFormat::MaskRGBA_Compressed);
            }

            auto assetPath = settings->ImportingTargetFolder + "/" + fileName;
            assetPath = AssetDatabase::GetUniquePath(assetPath);
            AssetDatabase::CreateAsset(asset, assetPath);
            importAssets.push_back(asset);
        }

        return importAssets;
    }
} // namespace pulsared