#include "Importers/FBXImporter.h"

#include "Pulsar/Prefab.h"
#include "Pulsar/Scene.h"

#include <Pulsar/Assets/NodeCollection.h>
#include <Pulsar/Assets/StaticMesh.h>
#include <Pulsar/Components/RendererComponent.h>
#include <Pulsar/Components/StaticMeshRendererComponent.h>
#include <PulsarEd/AssetDatabase.h>
#include <fbxsdk.h>

#ifdef IOS_REF
    #undef IOS_REF
    #define IOS_REF (*(pManager->GetIOSettings()))
#endif

namespace pulsared
{
    class FBXHelper
    {
        FBXHelper()
        {
            m_manager = FbxManager::Create();

            // Create an IOSettings object. This object holds all import/export settings.
            FbxIOSettings* ios = FbxIOSettings::Create(m_manager, IOSROOT);
            m_manager->SetIOSettings(ios);

            // Load plugins from the executable directory (optional)
            FbxString lPath = FbxGetApplicationDirectory();
            m_manager->LoadPluginsDirectory(lPath.Buffer());
        }
        FbxScene* LoadScene(const char* pFilename)
        {
            auto pScene = FbxScene::Create(m_manager, "My Scene");
            auto pManager = m_manager;

            int lFileMajor, lFileMinor, lFileRevision;
            int lSDKMajor, lSDKMinor, lSDKRevision;
            // int lFileFormat = -1;
            int lAnimStackCount;
            bool lStatus;
            char lPassword[1024];

            // Get the file version number generate by the FBX SDK.
            FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

            // Create an importer.
            FbxImporter* lImporter = FbxImporter::Create(pManager, "");

            // Initialize the importer by providing a filename.
            const bool lImportStatus = lImporter->Initialize(pFilename, -1, pManager->GetIOSettings());
            lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

            if (!lImportStatus)
            {
                FbxString error = lImporter->GetStatus().GetErrorString();
                throw std::runtime_error(string("FbxImporter::Initialize() failed: ") + error.Buffer());
            }

            // Import the scene.
            lStatus = lImporter->Import(pScene);
            if (lStatus == true)
            {
                // Check the scene integrity!
                FbxStatus status;
                FbxArray<FbxString*> details;
                FbxSceneCheckUtility sceneCheck(FbxCast<FbxScene>(pScene), &status, &details);
                lStatus = sceneCheck.Validate(FbxSceneCheckUtility::eCkeckData);
                bool lNotify = (!lStatus && details.GetCount() > 0) || (lImporter->GetStatus().GetCode() != FbxStatus::eSuccess);
                if (lNotify)
                {
                    if (details.GetCount())
                    {
                        FBXSDK_printf("Scene integrity verification failed with the following errors:\n");
                        for (int i = 0; i < details.GetCount(); i++)
                            FBXSDK_printf("   %s\n", details[i]->Buffer());

                        FbxArrayDelete<FbxString*>(details);
                    }

                    if (lImporter->GetStatus().GetCode() != FbxStatus::eSuccess)
                    {
                        FBXSDK_printf("\n");
                        FBXSDK_printf("WARNING:\n");
                        FBXSDK_printf("   The importer was able to read the file but with errors.\n");
                        FBXSDK_printf("   Loaded scene may be incomplete.\n\n");
                        FBXSDK_printf("   Last error message:'%s'\n", lImporter->GetStatus().GetErrorString());
                    }
                }
            }

            if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
            {
                assert((false, "Please enter password: "));
            }

            // Destroy the importer.
            lImporter->Destroy();

            return pScene;
        }

        ~FBXHelper()
        {
            m_manager->Destroy();
        }

        FbxManager* m_manager;
    };

    static void InitializeSdkObjects(FbxManager*& pManager, FbxScene*& pScene)
    {
        // The first thing to do is to create the FBX Manager which is the object allocator for almost all the classes in the SDK
        pManager = FbxManager::Create();
        if (!pManager)
        {
            FBXSDK_printf("Error: Unable to create FBX Manager!\n");
            exit(1);
        }
        else
            FBXSDK_printf("Autodesk FBX SDK version %s\n", pManager->GetVersion());

        // Create an IOSettings object. This object holds all import/export settings.
        FbxIOSettings* ios = FbxIOSettings::Create(pManager, IOSROOT);
        pManager->SetIOSettings(ios);

        // Load plugins from the executable directory (optional)
        FbxString lPath = FbxGetApplicationDirectory();
        pManager->LoadPluginsDirectory(lPath.Buffer());

        // Create an FBX scene. This object holds most objects imported/exported from/to files.
        pScene = FbxScene::Create(pManager, "My Scene");

        if (!pScene)
        {
            FBXSDK_printf("Error: Unable to create FBX scene!\n");
            exit(1);
        }
    }
    static bool LoadScene(FbxManager* pManager, FbxDocument* pScene, const char* pFilename)
    {
        int lFileMajor, lFileMinor, lFileRevision;
        int lSDKMajor, lSDKMinor, lSDKRevision;
        // int lFileFormat = -1;
        int lAnimStackCount;
        bool lStatus;
        char lPassword[1024];

        // Get the file version number generate by the FBX SDK.
        FbxManager::GetFileFormatVersion(lSDKMajor, lSDKMinor, lSDKRevision);

        // Create an importer.
        FbxImporter* lImporter = FbxImporter::Create(pManager, "");

        // Initialize the importer by providing a filename.
        const bool lImportStatus = lImporter->Initialize(pFilename, -1, pManager->GetIOSettings());
        lImporter->GetFileVersion(lFileMajor, lFileMinor, lFileRevision);

        if (!lImportStatus)
        {
            FbxString error = lImporter->GetStatus().GetErrorString();
            FBXSDK_printf("Call to FbxImporter::Initialize() failed.\n");
            FBXSDK_printf("Error returned: %s\n\n", error.Buffer());

            if (lImporter->GetStatus().GetCode() == FbxStatus::eInvalidFileVersion)
            {
                FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);
                FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);
            }

            return false;
        }

        FBXSDK_printf("FBX file format version for this FBX SDK is %d.%d.%d\n", lSDKMajor, lSDKMinor, lSDKRevision);

        if (lImporter->IsFBX())
        {
            FBXSDK_printf("FBX file format version for file '%s' is %d.%d.%d\n\n", pFilename, lFileMajor, lFileMinor, lFileRevision);

            // From this point, it is possible to access animation stack information without
            // the expense of loading the entire file.

            FBXSDK_printf("Animation Stack Information\n");

            lAnimStackCount = lImporter->GetAnimStackCount();

            FBXSDK_printf("    Number of Animation Stacks: %d\n", lAnimStackCount);
            FBXSDK_printf("    Current Animation Stack: \"%s\"\n", lImporter->GetActiveAnimStackName().Buffer());
            FBXSDK_printf("\n");

            for (int i = 0; i < lAnimStackCount; i++)
            {
                FbxTakeInfo* lTakeInfo = lImporter->GetTakeInfo(i);

                FBXSDK_printf("    Animation Stack %d\n", i);
                FBXSDK_printf("         Name: \"%s\"\n", lTakeInfo->mName.Buffer());
                FBXSDK_printf("         Description: \"%s\"\n", lTakeInfo->mDescription.Buffer());

                // Change the value of the import name if the animation stack should be imported
                // under a different name.
                FBXSDK_printf("         Import Name: \"%s\"\n", lTakeInfo->mImportName.Buffer());

                // Set the value of the import state to false if the animation stack should be not
                // be imported.
                FBXSDK_printf("         Import State: %s\n", lTakeInfo->mSelect ? "true" : "false");
                FBXSDK_printf("\n");
            }

            // Set the import states. By default, the import states are always set to
            // true. The code below shows how to change these states.
            IOS_REF.SetBoolProp(IMP_FBX_MATERIAL, true);
            IOS_REF.SetBoolProp(IMP_FBX_TEXTURE, true);
            IOS_REF.SetBoolProp(IMP_FBX_LINK, true);
            IOS_REF.SetBoolProp(IMP_FBX_SHAPE, true);
            IOS_REF.SetBoolProp(IMP_FBX_GOBO, true);
            IOS_REF.SetBoolProp(IMP_FBX_ANIMATION, true);
            IOS_REF.SetBoolProp(IMP_FBX_GLOBAL_SETTINGS, true);
        }

        // Import the scene.
        lStatus = lImporter->Import(pScene);
        if (lStatus == true)
        {
            // Check the scene integrity!
            FbxStatus status;
            FbxArray<FbxString*> details;
            FbxSceneCheckUtility sceneCheck(FbxCast<FbxScene>(pScene), &status, &details);
            lStatus = sceneCheck.Validate(FbxSceneCheckUtility::eCkeckData);
            bool lNotify = (!lStatus && details.GetCount() > 0) || (lImporter->GetStatus().GetCode() != FbxStatus::eSuccess);
            if (lNotify)
            {
                FBXSDK_printf("\n");
                FBXSDK_printf("********************************************************************************\n");
                if (details.GetCount())
                {
                    FBXSDK_printf("Scene integrity verification failed with the following errors:\n");
                    for (int i = 0; i < details.GetCount(); i++)
                        FBXSDK_printf("   %s\n", details[i]->Buffer());

                    FbxArrayDelete<FbxString*>(details);
                }

                if (lImporter->GetStatus().GetCode() != FbxStatus::eSuccess)
                {
                    FBXSDK_printf("\n");
                    FBXSDK_printf("WARNING:\n");
                    FBXSDK_printf("   The importer was able to read the file but with errors.\n");
                    FBXSDK_printf("   Loaded scene may be incomplete.\n\n");
                    FBXSDK_printf("   Last error message:'%s'\n", lImporter->GetStatus().GetErrorString());
                }
                FBXSDK_printf("********************************************************************************\n");
                FBXSDK_printf("\n");
            }
        }

        if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
        {
            FBXSDK_printf("Please enter password: ");

            lPassword[0] = '\0';

            FBXSDK_CRT_SECURE_NO_WARNING_BEGIN
            scanf("%s", lPassword);
            FBXSDK_CRT_SECURE_NO_WARNING_END

            FbxString lString(lPassword);

            IOS_REF.SetStringProp(IMP_FBX_PASSWORD, lString);
            IOS_REF.SetBoolProp(IMP_FBX_PASSWORD_ENABLE, true);

            lStatus = lImporter->Import(pScene);

            if (lStatus == false && lImporter->GetStatus().GetCode() == FbxStatus::ePasswordError)
            {
                FBXSDK_printf("\nPassword is wrong, import aborted.\n");
            }
        }

        // Destroy the importer.
        lImporter->Destroy();

        return lStatus;
    }
    static void DestroySdkObjects(FbxManager* pManager, bool pExitStatus)
    {
        // Delete the FBX Manager. All the objects that have been allocated using the FBX Manager and that haven't been explicitly destroyed are also automatically destroyed.
        if (pManager)
            pManager->Destroy();
        if (pExitStatus)
            FBXSDK_printf("Program Success!\n");
    }

    static inline Vector3f ToVector3f(const FbxVector4& vec)
    {
        return {static_cast<float>(vec[0]), static_cast<float>(vec[1]), static_cast<float>(vec[2])};
    }
    static inline Vector3f ToVector3f(const FbxDouble3& vec)
    {
        return {static_cast<float>(vec[0]), static_cast<float>(vec[1]), static_cast<float>(vec[2])};
    }
    static inline Vector2f ToVector2f(const FbxVector2& vec)
    {
        return {(float)vec[0], (float)vec[1]};
    }
    static inline auto ToColor(const FbxColor& color)
    {
        return Color4b{uint8_t(color.mRed * 255), uint8_t(color.mGreen * 255), uint8_t(color.mBlue * 255), uint8_t(color.mAlpha * 255) };
    }
    static inline Quat4f ToQuat(const FbxQuaternion& q)
    {
        return {(float)q[0], (float)q[1], (float)q[2], (float)q[3]};
    }

    template <typename T>
    static T GetColorLayerElement(FbxLayerElementTemplate<T>* layer, int ctrlPoint, int vertIndex)
    {
        const auto mappingMode   = layer->GetMappingMode();
        const auto referenceMode = layer->GetReferenceMode();
        switch (mappingMode)
        {
        case FbxGeometryElement::EMappingMode::eByControlPoint: {
            switch (referenceMode)
            {
            case FbxGeometryElement::EReferenceMode::eDirect: {
                return layer->GetDirectArray().GetAt(ctrlPoint);
                break;
            }
            case FbxGeometryElement::EReferenceMode::eIndexToDirect: {
                auto index = layer->GetIndexArray().GetAt(ctrlPoint);
                return layer->GetDirectArray().GetAt(index);
                break;
            }
            default:
                assert(false);
                break;
            }

            break;
        }
        case FbxGeometryElement::EMappingMode::eByPolygonVertex: {
            switch (referenceMode)
            {
            case FbxGeometryElement::EReferenceMode::eDirect: {
                return layer->GetDirectArray().GetAt(vertIndex);
                break;
            }
            case FbxGeometryElement::EReferenceMode::eIndexToDirect: {
                auto index = layer->GetIndexArray().GetAt(vertIndex);
                return layer->GetDirectArray().GetAt(index);
                break;
            }
            default:
                assert(false);
            }

            break;
        }
        default:
            assert(false);
        }
        return {};
    }

    template <typename T>
    static T GetUVLayerElement(FbxLayerElementTemplate<T>* layer, int ctrlPoint, int vertIndex)
    {
        const auto mappingMode   = layer->GetMappingMode();
        const auto referenceMode = layer->GetReferenceMode();
        switch (mappingMode)
        {
        case FbxGeometryElement::EMappingMode::eByControlPoint: {
            switch (referenceMode)
            {
            case FbxGeometryElement::EReferenceMode::eDirect: {
                return layer->GetDirectArray().GetAt(ctrlPoint);
                break;
            }
            case FbxGeometryElement::EReferenceMode::eIndexToDirect: {
                auto index = layer->GetIndexArray().GetAt(ctrlPoint);
                return layer->GetDirectArray().GetAt(index);
                break;
            }
            default:
                assert(false);
                break;
            }

            break;
        }
        case FbxGeometryElement::EMappingMode::eByPolygonVertex: {
            switch (referenceMode)
            {
            case FbxGeometryElement::EReferenceMode::eDirect:
            case FbxGeometryElement::EReferenceMode::eIndexToDirect: {
                return layer->GetDirectArray().GetAt(vertIndex);
                break;
            }
            default:
                assert(false);
            }

            break;
        }
        default:
            assert(false);
        }
        return {};
    }

    static RCPtr<StaticMesh> ProcessMesh(FbxNode* fbxNode, bool inverseCoordsystem)
    {
        const auto name = fbxNode->GetName();

        array_list<StaticMeshSection> sections;
        array_list<string> materialNames;

        materialNames.reserve(fbxNode->GetMaterialCount());
        for (int i = 0; i < fbxNode->GetMaterialCount(); i++)
        {
            materialNames.push_back(fbxNode->GetMaterial(i)->GetName());
        }

        const auto attrCount = fbxNode->GetNodeAttributeCount();
        for (int attrIndex = 0; attrIndex < attrCount; attrIndex++)
        {
            auto attr = fbxNode->GetNodeAttributeByIndex(attrIndex);
            if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
            {
                StaticMeshSection section;

                auto fbxMesh = static_cast<FbxMesh*>(attr);
                assert(fbxMesh);

                constexpr int kPolygonCount = 3;
                const auto vertexCount = fbxMesh->GetPolygonVertexCount();
                const auto polygonCount = fbxMesh->GetPolygonCount();
                assert(vertexCount == polygonCount * kPolygonCount);

                // 确定实际 UV 套数
                const uint8_t numUV = (uint8_t)std::min(fbxMesh->GetUVLayerCount(), (int)STATICMESH_MAX_TEXTURE_COORDS);
                section.NumTexCoords = numUV;

                // 初始化各属性数组
                section.Indices.resize(vertexCount);
                section.Positions.resize(vertexCount);
                section.Normals.resize(vertexCount);
                section.TexCoords.resize(numUV);
                for (uint8_t uvIdx = 0; uvIdx < numUV; uvIdx++)
                    section.TexCoords[uvIdx].resize(vertexCount);

                const bool hasColors   = fbxMesh->GetLayer(0) && fbxMesh->GetLayer(0)->GetVertexColors();
                const bool hasTangents = fbxMesh->GetElementTangentCount() > 0 && fbxMesh->GetElementBinormalCount() > 0;
                if (hasColors)   section.Colors.resize(vertexCount);
                if (hasTangents) section.Tangents.resize(vertexCount);

                // #pragma omp parallel for
                for (int polyIndex = 0; polyIndex < polygonCount; polyIndex++)
                {
                    for (int vertIndexInFace = 0; vertIndexInFace < kPolygonCount; vertIndexInFace++)
                    {
                        const auto vertexIndex       = polyIndex * kPolygonCount + vertIndexInFace;
                        const auto controlPointIndex = fbxMesh->GetPolygonVertex(polyIndex, vertIndexInFace);

                        // position
                        section.Positions[vertexIndex] = ToVector3f(fbxMesh->GetControlPointAt(controlPointIndex));

                        // normal
                        FbxVector4 normal;
                        fbxMesh->GetPolygonVertexNormal(polyIndex, vertIndexInFace, normal);
                        section.Normals[vertexIndex] = ToVector3f(normal);

                        // tangent + bitangent sign (w)
                        if (hasTangents)
                        {
                            const Vector3f T = ToVector3f(GetColorLayerElement(fbxMesh->GetElementTangent(0),  controlPointIndex, vertexIndex));
                            const Vector3f B = ToVector3f(GetColorLayerElement(fbxMesh->GetElementBinormal(0), controlPointIndex, vertexIndex));
                            const Vector3f N = section.Normals[vertexIndex];
                            // cross(N,T) 与 FBX 提供的 B 方向一致则 w=+1，否则 w=-1
                            const float w = Dot(Cross(N, T), B) > 0.0f ? 1.0f : -1.0f;
                            section.Tangents[vertexIndex] = Vector4f{T.x, T.y, T.z, w};
                        }

                        // UVs
                        for (uint8_t uvIdx = 0; uvIdx < numUV; uvIdx++)
                        {
                            auto uv = ToVector2f(GetUVLayerElement(fbxMesh->GetElementUV(uvIdx), controlPointIndex, vertexIndex));
                            if (inverseCoordsystem) uv.y = 1.0f - uv.y;
                            section.TexCoords[uvIdx][vertexIndex] = uv;
                        }

                        // color
                        if (hasColors)
                        {
                            section.Colors[vertexIndex] = ToColor(GetColorLayerElement(
                                fbxMesh->GetLayer(0)->GetVertexColors(), controlPointIndex, vertexIndex));
                        }

                        auto indicesValue = vertexIndex;
                        if (inverseCoordsystem)
                        {
                            if (vertIndexInFace == 1)
                                indicesValue += 1;
                            if (vertIndexInFace == 2)
                                indicesValue -= 1;
                        }
                        section.Indices[vertexIndex] = indicesValue;
                    }
                }

                section.MaterialIndex = attrIndex;

                sections.push_back(std::move(section));
            }
        }
        if (sections.empty())
        {
            return nullptr;
        }
        return StaticMesh::StaticCreate(name, std::move(sections), std::move(materialNames));
    }

    static void ProcessNode(
        FbxNode* fbxNode,
        ObjectPtr<Node> parentNode,
        RCPtr<Prefab> pscene,
        FBXImporterSettings* settings,
        bool inverseCoordsystem,
        const string& meshFolder,
        array_list<RCPtr<AssetObject>>& importedAssets
        )
    {
        auto newNodeName = fbxNode->GetName();
        const auto newNode = pscene->NewNode(newNodeName, parentNode);

        if (auto staticMesh = ProcessMesh(fbxNode, inverseCoordsystem))
        {
            const auto meshPath = meshFolder + "/" + staticMesh->GetName();
            AssetDatabase::CreateAsset(staticMesh, meshPath);
            newNode->AddComponent<StaticMeshRendererComponent>()->SetStaticMesh(staticMesh);
            auto translation = ToVector3f(fbxNode->LclTranslation.Get());
            auto scaling = ToVector3f(fbxNode->LclScaling.Get());

            FbxEuler::EOrder order{};
            fbxNode->GetRotationOrder(FbxNode::EPivotSet::eSourcePivot, order);
            assert(order == FbxEuler::EOrder::eOrderXYZ);
            FbxQuaternion q;
            q.ComposeSphericalXYZ(FbxVector4(fbxNode->LclRotation.Get()));
            auto rotation = ToQuat(q);

            auto transform = newNode->GetTransform();
            transform->SetPosition(translation);
            transform->SetRotation(rotation);
            transform->SetScale(scaling);

            importedAssets.push_back(staticMesh);
        }

        const auto childCount = fbxNode->GetChildCount();
        for (int childIndex = 0; childIndex < childCount; childIndex++)
        {
            const auto childFbxNode = fbxNode->GetChild(childIndex);
            ProcessNode(childFbxNode, newNode, pscene, settings, inverseCoordsystem, meshFolder, importedAssets);
        }
    }

    array_list<RCPtr<AssetObject>> FBXImporter::Import(AssetImporterSettings* settings)
    {
        FBXImporterSettings* fbxsetting = static_cast<FBXImporterSettings*>(settings);

        using namespace fbxsdk;
        FbxManager* fbxManager;
        FbxScene* fbxScene;

        array_list<RCPtr<AssetObject>> importedAssets;

        InitializeSdkObjects(fbxManager, fbxScene);

        for (auto& importFile : *settings->ImportFiles)
        {
            LoadScene(fbxManager, fbxScene, importFile.c_str());

            bool inverseCoordSystem = false;
            if (fbxsetting->ConvertAxisSystem)
            {
                const auto axisSystem = fbxScene->GetGlobalSettings().GetAxisSystem();
                if (axisSystem.GetCoorSystem() == FbxAxisSystem::eRightHanded)
                {
                    inverseCoordSystem = true;
                }
                const auto ourAxisSystem = FbxAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eLeftHanded);
                if (axisSystem != ourAxisSystem)
                {
                    ourAxisSystem.ConvertScene(fbxScene);
                }
            }

            auto originUnit = fbxScene->GetGlobalSettings().GetOriginalSystemUnit();
            auto unit = fbxScene->GetGlobalSettings().GetSystemUnit();

            if (unit != FbxSystemUnit::m)
            {
                FbxSystemUnit::m.ConvertScene(fbxScene);
            }

            FbxGeometryConverter geomConverter(fbxManager);
            geomConverter.Triangulate(fbxScene, true);
            geomConverter.SplitMeshesPerMaterial(fbxScene, true);

            const auto fbxRootNode = fbxScene->GetRootNode();
            const auto filename = PathUtil::GetFilenameWithoutExt(importFile);
            auto prefab = Prefab::StaticCreate(filename);
            const auto targetMeshFolder = settings->ImportingTargetFolder + "/" + filename + "_Items";
            const auto rootCount = fbxRootNode->GetChildCount();
            for (int i = 0; i < rootCount; ++i)
            {
                auto sceneRootNode = fbxRootNode->GetChild(i);
                ProcessNode(sceneRootNode, nullptr, prefab, fbxsetting, inverseCoordSystem, targetMeshFolder, importedAssets);
            }

            AssetDatabase::CreateAsset(prefab, settings->ImportingTargetFolder + "/" + filename);
            importedAssets.push_back(prefab);

            DestroySdkObjects(fbxManager, 0);
        }
        return importedAssets;
    }

} // namespace pulsared
