#pragma once
#include "AssetImporter.h"

namespace pulsared
{
    class PULSARED_API ImageImporterSettings : public AssetImporterSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ImageImporterSettings, AssetImporterSettings);
    public:
        CORELIB_REFL_DECL_FIELD(FindMaterial);
        bool FindMaterial = true;

        CORELIB_REFL_DECL_FIELD(ConvertAxisSystem);
        bool ConvertAxisSystem = true;
    };

    class PULSARED_API ImageImporter : public AssetImporter
    {
    public:
        //static Node_ref Import(string_view path, FBXImporterSettings* settings);
        string GetImporterType() const override { return "ImageImporter"; }
        array_list<RCPtr<AssetObject>> Import(AssetImporterSettings* settings) override;
    };

    class PULSARED_API ImageImporterFactory final : public AssetImporterFactory
    {
        PULSARED_ASSET_IMPORTER_FACTORY(ImageImporterFactory);
    public:
        ImageImporterFactory()
        {
            m_supportedFormats.push_back(".jpg");
            m_supportedFormats.push_back(".png");
            m_supportedFormats.push_back(".bmp");
            m_supportedFormats.push_back(".tga");
            m_supportedFormats.push_back(".hdr");
        }
        string_view GetDescription() const override { return "ImageImporter"; }
        std::shared_ptr<AssetImporter> CreateImporter() override
        {
            return std::make_shared<ImageImporter>();
        }
        SPtr<AssetImporterSettings> CreateImporterSettings() override
        {
            return mksptr(new ImageImporterSettings);
        }
    };
}