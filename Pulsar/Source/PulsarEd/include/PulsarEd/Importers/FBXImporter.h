#pragma once
#include <PulsarEd/Assembly.h>
#include "AssetImporter.h"
#include "Pulsar/Node.h"

namespace pulsared
{
    class PULSARED_API FBXImporterSettings : public AssetImporterSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::FBXImporterSettings, AssetImporterSettings);
    public:
        CORELIB_REFL_DECL_FIELD(FindMaterial);
        bool FindMaterial = true;

        CORELIB_REFL_DECL_FIELD(ConvertAxisSystem);
        bool ConvertAxisSystem = true;
    };

    class PULSARED_API FBXImporter : public AssetImporter
    {
    public:
        //static Node_ref Import(string_view path, FBXImporterSettings* settings);
        string GetImporterType() const override { return "FBXImporter"; }
        array_list<AssetObject_ref> Import(AssetImporterSettings* settings) override;
    };

    class PULSARED_API FBXImporterFactory final : public AssetImporterFactory
    {
        PULSARED_ASSET_IMPORTER_FACTORY(FBXImporterFactory);
    public:
        FBXImporterFactory()
        {
            m_supportedFormats.push_back(".fbx");
        }
        string_view GetDescription() const override { return "FBXImporter"; }
        std::shared_ptr<AssetImporter> CreateImporter() override
        {
            return std::make_shared<FBXImporter>();
        }
        sptr<AssetImporterSettings> CreateImporterSettings() override
        {
            return mksptr(new FBXImporterSettings);
        }
    };


}
