#pragma once
#include <PulsarEd/Assembly.h>
#include "AssetImporter.h"
#include "Pulsar/Node.h"
#include "Pulsar/MatchPolicy.h"

namespace pulsared
{
    class PULSARED_API AssimpImporterSettings : public AssetImporterSettings
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::AssimpImporterSettings, AssetImporterSettings);
    public:
        CORELIB_REFL_DECL_FIELD(MaterialPolicy);
        pulsar::MatchPolicy MaterialPolicy = pulsar::MatchPolicy::MatchOrCreate;

        CORELIB_REFL_DECL_FIELD(RecomputeTangents);
        bool RecomputeTangents = true;

        CORELIB_REFL_DECL_FIELD(UseMikktspace);
        bool UseMikktspace = true;

        CORELIB_REFL_DECL_FIELD(SkinnedMeshToStaticMesh);
        bool SkinnedMeshToStaticMesh = false;

        CORELIB_REFL_DECL_FIELD(ImportAnimations);
        bool ImportAnimations = true;
    };

    class PULSARED_API AssimpImporter : public AssetImporter
    {
    public:
        string GetImporterType() const override { return "AssimpImporter"; }
        array_list<RCPtr<AssetObject>> Import(AssetImporterSettings* settings) override;
    };

    class PULSARED_API AssimpImporterFactory final : public AssetImporterFactory
    {
        PULSARED_ASSET_IMPORTER_FACTORY(AssimpImporterFactory);
    public:
        AssimpImporterFactory()
        {
            m_supportedFormats.push_back(".fbx");
            m_supportedFormats.push_back(".gltf");
            m_supportedFormats.push_back(".glb");
            m_supportedFormats.push_back(".obj");
            m_supportedFormats.push_back(".dae");
        }
        string_view GetDescription() const override { return "AssimpImporter"; }
        std::shared_ptr<AssetImporter> CreateImporter() override
        {
            return std::make_shared<AssimpImporter>();
        }
        SPtr<AssetImporterSettings> CreateImporterSettings() override
        {
            return mksptr(new AssimpImporterSettings);
        }
    };

} // namespace pulsared
