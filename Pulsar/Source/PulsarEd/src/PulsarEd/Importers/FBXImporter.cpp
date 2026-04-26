#include "Importers/FBXImporter.h"

#include "Pulsar/Prefab.h"
#include "Pulsar/Scene.h"

#include <Pulsar/Assets/NodeCollection.h>
#include <Pulsar/Assets/StaticMesh.h>
#include <Pulsar/Assets/SkinnedMesh.h>
#include <Pulsar/Assets/Skeleton.h>
#include <Pulsar/Assets/Material.h>
#include <CoreLib/UString.h>
#include <Pulsar/Components/RendererComponent.h>
#include <Pulsar/Components/StaticMeshRendererComponent.h>
#include <Pulsar/Components/SkinnedMeshRendererComponent.h>
#include <Pulsar/TransformUtil.h>
#include <Pulsar/AssetManager.h>
#include <PulsarEd/AssetDatabase.h>
#include <fbxsdk.h>
#include <mikktspace.h>

#ifdef IOS_REF
    #undef IOS_REF
    #define IOS_REF (*(pManager->GetIOSettings()))
#endif

namespace pulsared
{
    // -----------------------------------------------------------------------
    // Mikktspace 切线生成
    // -----------------------------------------------------------------------
    struct MikkTangentGenData
    {
        const array_list<Vector3f>* Positions;
        const array_list<Vector3f>* Normals;
        const array_list<Vector2f>* UVs;
        array_list<Vector4f>*       Tangents;
    };

    static int MikkGetNumFaces(const SMikkTSpaceContext* pContext)
    {
        auto* data = static_cast<const MikkTangentGenData*>(pContext->m_pUserData);
        return static_cast<int>(data->Positions->size() / 3);
    }

    static int MikkGetNumVerticesOfFace(const SMikkTSpaceContext* pContext, const int iFace)
    {
        return 3;
    }

    static void MikkGetPosition(const SMikkTSpaceContext* pContext, float fvPosOut[], const int iFace, const int iVert)
    {
        auto* data = static_cast<const MikkTangentGenData*>(pContext->m_pUserData);
        const Vector3f& p = (*data->Positions)[iFace * 3 + iVert];
        fvPosOut[0] = p.x; fvPosOut[1] = p.y; fvPosOut[2] = p.z;
    }

    static void MikkGetNormal(const SMikkTSpaceContext* pContext, float fvNormOut[], const int iFace, const int iVert)
    {
        auto* data = static_cast<const MikkTangentGenData*>(pContext->m_pUserData);
        const Vector3f& n = (*data->Normals)[iFace * 3 + iVert];
        fvNormOut[0] = n.x; fvNormOut[1] = n.y; fvNormOut[2] = n.z;
    }

    static void MikkGetTexCoord(const SMikkTSpaceContext* pContext, float fvTexcOut[], const int iFace, const int iVert)
    {
        auto* data = static_cast<const MikkTangentGenData*>(pContext->m_pUserData);
        const Vector2f& uv = (*data->UVs)[iFace * 3 + iVert];
        fvTexcOut[0] = uv.x; fvTexcOut[1] = uv.y;
    }

    static void MikkSetTSpaceBasic(const SMikkTSpaceContext* pContext, const float fvTangent[], const float fSign, const int iFace, const int iVert)
    {
        auto* data = static_cast<MikkTangentGenData*>(pContext->m_pUserData);
        (*data->Tangents)[iFace * 3 + iVert] = Vector4f{fvTangent[0], fvTangent[1], fvTangent[2], fSign};
    }

    static void GenerateTangentsForSection(
        const array_list<Vector3f>& positions,
        const array_list<Vector3f>& normals,
        const array_list<Vector2f>& uvs,
        array_list<Vector4f>& outTangents)
    {
        if (uvs.empty()) return;
        if (positions.empty()) return;

        outTangents.resize(positions.size());

        MikkTangentGenData data{&positions, &normals, &uvs, &outTangents};

        SMikkTSpaceInterface iface{};
        iface.m_getNumFaces          = MikkGetNumFaces;
        iface.m_getNumVerticesOfFace = MikkGetNumVerticesOfFace;
        iface.m_getPosition          = MikkGetPosition;
        iface.m_getNormal            = MikkGetNormal;
        iface.m_getTexCoord          = MikkGetTexCoord;
        iface.m_setTSpaceBasic       = MikkSetTSpaceBasic;

        SMikkTSpaceContext ctx{};
        ctx.m_pInterface = &iface;
        ctx.m_pUserData  = &data;

        genTangSpaceDefault(&ctx);
    }

    // -----------------------------------------------------------------------
    // 简化版切线生成（逐三角计算 + 顶点平均）
    // -----------------------------------------------------------------------
    static void GenerateSimpleTangentsForSection(
        const array_list<Vector3f>& positions,
        const array_list<Vector3f>& normals,
        const array_list<Vector2f>& uvs,
        array_list<Vector4f>& outTangents)
    {
        if (uvs.empty() || positions.empty()) return;

        const size_t vertCount = positions.size();
        outTangents.resize(vertCount);
        array_list<Vector3f> accumT(vertCount);
        array_list<float>    accumSign(vertCount, 0.0f);

        const size_t triCount = vertCount / 3;
        for (size_t tri = 0; tri < triCount; ++tri)
        {
            const size_t i0 = tri * 3 + 0;
            const size_t i1 = tri * 3 + 1;
            const size_t i2 = tri * 3 + 2;

            const Vector3f e1  = positions[i1] - positions[i0];
            const Vector3f e2  = positions[i2] - positions[i0];
            const Vector2f duv1 = uvs[i1] - uvs[i0];
            const Vector2f duv2 = uvs[i2] - uvs[i0];

            const float det = duv1.x * duv2.y - duv1.y * duv2.x;
            if (fabsf(det) < FLT_EPSILON) continue;

            const float r = 1.0f / det;
            const Vector3f T = (e1 * duv2.y - e2 * duv1.y) * r;
            const Vector3f B = (e2 * duv1.x - e1 * duv2.x) * r;

            for (int j = 0; j < 3; ++j)
            {
                const size_t idx = tri * 3 + j;
                accumT[idx] = accumT[idx] + T;
                const float w = Dot(Cross(normals[idx], T), B) > 0.0f ? 1.0f : -1.0f;
                accumSign[idx] += w;
            }
        }

        for (size_t i = 0; i < vertCount; ++i)
        {
            Vector3f T = accumT[i];
            if (Dot(T, T) > FLT_EPSILON)
            {
                T = Normalize(T);
                const Vector3f N = normals[i];
                T = Normalize(T - N * Dot(N, T)); // Gram-Schmidt
            }
            else
            {
                T = Vector3f{1, 0, 0};
            }
            const float w = accumSign[i] >= 0.0f ? 1.0f : -1.0f;
            outTangents[i] = Vector4f{T.x, T.y, T.z, w};
        }
    }

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

            lAnimStackCount = lImporter->GetAnimStackCount();

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

    // -----------------------------------------------------------------------
    // ProcessMesh（StaticMesh）
    // -----------------------------------------------------------------------
    static RCPtr<StaticMesh> ProcessMesh(FbxNode* fbxNode, bool inverseCoordsystem, bool recomputeTangents, bool useMikktspace)
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
                if (hasColors) section.Colors.resize(vertexCount);
                if (recomputeTangents || hasTangents) section.Tangents.resize(vertexCount);

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
                        if (!recomputeTangents && hasTangents)
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

                if (recomputeTangents && numUV > 0)
                {
                    if (useMikktspace)
                        GenerateTangentsForSection(section.Positions, section.Normals, section.TexCoords[0], section.Tangents);
                    else
                        GenerateSimpleTangentsForSection(section.Positions, section.Normals, section.TexCoords[0], section.Tangents);
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

    static bool HasSkinDeformer(FbxMesh* fbxMesh)
    {
        return fbxMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
    }

    static RCPtr<SkinnedMesh> ProcessSkinnedMesh(FbxNode* fbxNode, RCPtr<Skeleton> skeleton, bool inverseCoordsystem, bool recomputeTangents, bool useMikktspace)
    {
        const auto name = fbxNode->GetName();

        array_list<SkinnedMeshSection> sections;
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
                auto fbxMesh = static_cast<FbxMesh*>(attr);
                assert(fbxMesh);

                // ------------------------------------------------------------------
                // 1. 收集 skinning 数据（按 control point）
                // ------------------------------------------------------------------
                array_list<array_list<std::pair<int, float>>> cpInfluences;
                cpInfluences.resize(fbxMesh->GetControlPointsCount());

                int skinCount = fbxMesh->GetDeformerCount(FbxDeformer::eSkin);
                for (int skinIndex = 0; skinIndex < skinCount; ++skinIndex)
                {
                    FbxSkin* skin = (FbxSkin*)fbxMesh->GetDeformer(skinIndex, FbxDeformer::eSkin);
                    int clusterCount = skin->GetClusterCount();
                    for (int clusterIndex = 0; clusterIndex < clusterCount; ++clusterIndex)
                    {
                        FbxCluster* cluster = skin->GetCluster(clusterIndex);
                        FbxNode* linkNode = cluster->GetLink();
                        if (!linkNode) continue;

                        string boneName = linkNode->GetName();
                        int boneIndex = skeleton->FindBoneIndex(boneName);
                        if (boneIndex < 0)
                        {
                            Logger::Log(StringUtil::Concat("Warning: SkinnedMesh bone '", boneName, "' not found in Skeleton"));
                            continue;
                        }

                        int* indices = cluster->GetControlPointIndices();
                        double* weights = cluster->GetControlPointWeights();
                        int indexCount = cluster->GetControlPointIndicesCount();

                        for (int i = 0; i < indexCount; ++i)
                        {
                            int cpIndex = indices[i];
                            float weight = static_cast<float>(weights[i]);
                            if (cpIndex >= 0 && cpIndex < (int)cpInfluences.size() && weight > 0.0f)
                            {
                                cpInfluences[cpIndex].push_back({boneIndex, weight});
                            }
                        }
                    }
                }

                // 排序、截断到4个、归一化
                for (auto& influences : cpInfluences)
                {
                    std::sort(influences.begin(), influences.end(), [](const auto& a, const auto& b) {
                        return a.second > b.second;
                    });
                    if (influences.size() > SKINNEDMESH_MAX_BONE_INFLUENCES)
                    {
                        influences.resize(SKINNEDMESH_MAX_BONE_INFLUENCES);
                    }
                    float sum = 0.0f;
                    for (const auto& [idx, w] : influences) sum += w;
                    if (sum > 0.0f)
                    {
                        for (auto& [idx, w] : influences) w /= sum;
                    }
                }

                // ------------------------------------------------------------------
                // 2. 处理几何数据（与 ProcessMesh 相同）
                // ------------------------------------------------------------------
                SkinnedMeshSection section;

                constexpr int kPolygonCount = 3;
                const auto vertexCount = fbxMesh->GetPolygonVertexCount();
                const auto polygonCount = fbxMesh->GetPolygonCount();
                assert(vertexCount == polygonCount * kPolygonCount);

                const uint8_t numUV = (uint8_t)std::min(fbxMesh->GetUVLayerCount(), (int)STATICMESH_MAX_TEXTURE_COORDS);
                section.NumTexCoords = numUV;

                section.Indices.resize(vertexCount);
                section.Positions.resize(vertexCount);
                section.Normals.resize(vertexCount);
                section.TexCoords.resize(numUV);
                for (uint8_t uvIdx = 0; uvIdx < numUV; uvIdx++)
                    section.TexCoords[uvIdx].resize(vertexCount);

                const bool hasColors   = fbxMesh->GetLayer(0) && fbxMesh->GetLayer(0)->GetVertexColors();
                const bool hasTangents = fbxMesh->GetElementTangentCount() > 0 && fbxMesh->GetElementBinormalCount() > 0;
                if (hasColors) section.Colors.resize(vertexCount);
                if (recomputeTangents || hasTangents) section.Tangents.resize(vertexCount);

                // 蒙皮数据数组
                section.BoneIndices.resize(vertexCount);
                section.BoneWeights.resize(vertexCount);

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
                        if (!recomputeTangents && hasTangents)
                        {
                            const Vector3f T = ToVector3f(GetColorLayerElement(fbxMesh->GetElementTangent(0),  controlPointIndex, vertexIndex));
                            const Vector3f B = ToVector3f(GetColorLayerElement(fbxMesh->GetElementBinormal(0), controlPointIndex, vertexIndex));
                            const Vector3f N = section.Normals[vertexIndex];
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

                        // indices
                        auto indicesValue = vertexIndex;
                        if (inverseCoordsystem)
                        {
                            if (vertIndexInFace == 1)
                                indicesValue += 1;
                            if (vertIndexInFace == 2)
                                indicesValue -= 1;
                        }
                        section.Indices[vertexIndex] = indicesValue;

                        // skinning data
                        const auto& influences = cpInfluences[controlPointIndex];
                        std::array<uint32_t, SKINNEDMESH_MAX_BONE_INFLUENCES> boneIndices = {};
                        std::array<float,    SKINNEDMESH_MAX_BONE_INFLUENCES> boneWeights = {};
                        for (size_t i = 0; i < influences.size() && i < SKINNEDMESH_MAX_BONE_INFLUENCES; ++i)
                        {
                            boneIndices[i] = static_cast<uint32_t>(influences[i].first);
                            boneWeights[i] = influences[i].second;
                        }
                        section.BoneIndices[vertexIndex] = boneIndices;
                        section.BoneWeights[vertexIndex] = boneWeights;
                    }
                }

                if (recomputeTangents && numUV > 0)
                {
                    if (useMikktspace)
                        GenerateTangentsForSection(section.Positions, section.Normals, section.TexCoords[0], section.Tangents);
                    else
                        GenerateSimpleTangentsForSection(section.Positions, section.Normals, section.TexCoords[0], section.Tangents);
                }

                section.MaterialIndex = attrIndex;
                sections.push_back(std::move(section));
            }
        }

        if (sections.empty()) return nullptr;
        return SkinnedMesh::StaticCreate(name, skeleton, std::move(sections), std::move(materialNames));
    }

    // 根据 MaterialPolicy 为 renderer 的每个 material slot 匹配或创建材质
    static void ApplyMaterialPolicyImpl(
        size_t materialCount,
        const array_list<string>& materialNames,
        FBXImporterSettings* settings,
        const string& meshFolder,
        array_list<RCPtr<AssetObject>>& importedAssets,
        auto&& setMaterialFn)
    {
        const auto policy = settings->MaterialPolicy;
        if (policy == MatchPolicy::None)
            return;

        for (size_t i = 0; i < materialCount; ++i)
        {
            const string& matName = (i < materialNames.size()) ? materialNames[i] : string();

            RCPtr<Material> foundMaterial;

            if (policy == MatchPolicy::Match || policy == MatchPolicy::MatchOrCreate)
            {
                // 在当前导入文件夹中按名称查找材质
                auto paths = AssetDatabase::FindAssets(cltypeof<Material>(), meshFolder);
                for (auto& path : paths)
                {
                    auto assetName = AssetDatabase::AssetPathToAssetName(path);
                    if (assetName == matName)
                    {
                        foundMaterial = AssetManager::Get()->LoadAsset<Material>(path);
                        break;
                    }
                }
            }

            if (!foundMaterial && (policy == MatchPolicy::MatchOrCreate || policy == MatchPolicy::AlwaysCreate))
            {
                // 创建新材质（使用 Lambert shader）
                auto shader = AssetManager::Get()->LoadAsset<Shader>("Engine/Shaders/Lambert");
                if (shader)
                {
                    foundMaterial = Material::StaticCreate(shader, matName);
                    AssetDatabase::CreateAsset(foundMaterial, meshFolder + "/" + matName);
                    importedAssets.push_back(foundMaterial);
                }
            }

            if (foundMaterial)
            {
                setMaterialFn((int)i, foundMaterial);
            }
        }
    }

    // -----------------------------------------------------------------------
    // 辅助：提取节点 TRS 并应用到 transform
    // -----------------------------------------------------------------------
    static void ApplyNodeTransform(FbxNode* fbxNode, Node* node)
    {
        auto transform = node->GetTransform();
        // 使用 EvaluateLocalTransform 获取完整的 local 变换（包含 PreRotation/PostRotation/RotationOrder）
        const FbxAMatrix localMatrix = fbxNode->EvaluateLocalTransform(FBXSDK_TIME_ZERO);
        transform->SetPosition(ToVector3f(localMatrix.GetT()));
        transform->SetRotation(ToQuat(localMatrix.GetQ()));
        transform->SetScale(ToVector3f(localMatrix.GetS()));
    }

    static void ApplyNodeTransform(Node* node, const Vector3f& pos, const Quat4f& rot, const Vector3f& scale)
    {
        auto transform = node->GetTransform();
        transform->SetPosition(pos);
        transform->SetRotation(rot);
        transform->SetScale(scale);
    }

    static bool IsSkeletonNode(FbxNode* fbxNode)
    {
        for (int i = 0; i < fbxNode->GetNodeAttributeCount(); ++i)
        {
            if (fbxNode->GetNodeAttributeByIndex(i)->GetAttributeType() == FbxNodeAttribute::eSkeleton)
                return true;
        }
        return false;
    }

    static inline Matrix4f ToMatrix4f(const FbxAMatrix& m)
    {
        Matrix4f result;
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                result[i][j] = static_cast<float>(m.Get(j, i)); // FbxAMatrix::Get(row, col), Matrix4f[i][j]=col i row j
        return result;
    }

    // ConvertScene 通过翻转一个轴实现 RH->LH，导致提取出的旋转四元数在 Pulsar 的
    // 左手系中看起来反向。此修正仅对旋转四元数做符号调整：
    // 对于 X 轴反射（或等效操作），四元数修正为 (-x, y, -z, w)。
    // 我们不在矩阵层面做反射，因为那会把平移也翻转到错误的方向。
    static inline Quat4f FixRotationForLH(const Quat4f& q)
    {
        return Quat4f{-q.x, q.y, -q.z, q.w};
    }

    // -----------------------------------------------------------------------
    // Skeleton 导入
    // -----------------------------------------------------------------------
    struct FbxSkeletonNodeInfo
    {
        FbxNode* Node = nullptr;
        int32_t ParentIndex = -1;
    };

    static void CollectSkeletonNodesRecursive(FbxNode* node, array_list<FbxSkeletonNodeInfo>& outInfos, int32_t parentIndex)
    {
        bool isSkeleton = false;
        for (int i = 0; i < node->GetNodeAttributeCount(); ++i)
        {
            if (node->GetNodeAttributeByIndex(i)->GetAttributeType() == FbxNodeAttribute::eSkeleton)
            {
                isSkeleton = true;
                break;
            }
        }

        int32_t myIndex = parentIndex;
        if (isSkeleton)
        {
            myIndex = static_cast<int32_t>(outInfos.size());
            outInfos.push_back({ node, parentIndex });
        }

        for (int i = 0; i < node->GetChildCount(); ++i)
        {
            CollectSkeletonNodesRecursive(node->GetChild(i), outInfos, myIndex);
        }
    }

    static RCPtr<Skeleton> ProcessSkeleton(FbxScene* fbxScene, const string& name, bool inverseCoordSystem)
    {
        array_list<FbxSkeletonNodeInfo> skeletonNodes;
        CollectSkeletonNodesRecursive(fbxScene->GetRootNode(), skeletonNodes, -1);

        if (skeletonNodes.empty())
            return nullptr;

        // 确定 skeleton root（ParentIndex == -1 的第一个节点）
        int32_t skeletonRootIndex = -1;
        for (int32_t i = 0; i < (int32_t)skeletonNodes.size(); ++i)
        {
            if (skeletonNodes[i].ParentIndex == -1)
            {
                skeletonRootIndex = i;
                break;
            }
        }
        if (skeletonRootIndex < 0)
            skeletonRootIndex = 0;

        FbxNode* skeletonRootNode = skeletonNodes[skeletonRootIndex].Node;
        const FbxAMatrix rootGlobalBind = skeletonRootNode->EvaluateGlobalTransform(FBXSDK_TIME_ZERO);
        const FbxAMatrix rootGlobalBindInv = rootGlobalBind.Inverse();

        array_list<BoneInfo> bones;
        bones.reserve(skeletonNodes.size());

        // 第一遍：提取 Name/Path/ParentIndex 和 Local TRS（从 FbxAMatrix）
        for (int32_t i = 0; i < (int32_t)skeletonNodes.size(); ++i)
        {
            const auto& info = skeletonNodes[i];
            FbxNode* boneNode = info.Node;

            BoneInfo bone;
            bone.Name = boneNode->GetName();

            // 构建 Path：从 skeleton root 到当前节点的路径
            {
                array_list<string> pathParts;
                int32_t idx = i;
                while (idx >= 0)
                {
                    pathParts.push_back(skeletonNodes[idx].Node->GetName());
                    idx = skeletonNodes[idx].ParentIndex;
                }
                std::reverse(pathParts.begin(), pathParts.end());
                bone.Path = StringUtil::Join(pathParts, "/");
            }

            bone.ParentIndex = info.ParentIndex;

            // Global bind pose
            const FbxAMatrix globalBind = boneNode->EvaluateGlobalTransform(FBXSDK_TIME_ZERO);

            // BindModelMatrix：相对于 skeleton root 的 canonical model space
            const FbxAMatrix bindModelMatrixFbx = rootGlobalBindInv * globalBind;

            // 重建 Local TRS（全部在 FbxAMatrix 中计算）
            FbxAMatrix localMatrix;
            if (bone.ParentIndex >= 0)
            {
                FbxAMatrix parentGlobalBind = skeletonNodes[bone.ParentIndex].Node->EvaluateGlobalTransform(FBXSDK_TIME_ZERO);
                FbxAMatrix parentBindModel = rootGlobalBindInv * parentGlobalBind;
                localMatrix = parentBindModel.Inverse() * bindModelMatrixFbx;
            }
            else
            {
                localMatrix = bindModelMatrixFbx;
            }

            bone.LocalTranslation = ToVector3f(localMatrix.GetT());
            bone.LocalRotation = ToQuat(localMatrix.GetQ());
            bone.LocalScale = ToVector3f(localMatrix.GetS());

            // 如果进行了 RH->LH 坐标系转换，对局部旋转做修正。
            // ConvertScene 在改变 handness 时会翻转一个轴，导致提取出的旋转
            // 在 Pulsar 的左手系中看起来反向。修正为 (-x, y, -z, w)。
            if (inverseCoordSystem)
            {
                bone.LocalRotation = FixRotationForLH(bone.LocalRotation);
            }

            bones.push_back(bone);
        }

        // 第二遍：用修正后的 Local TRS 重新计算 BindModelMatrix，确保与 LocalRotation 自洽
        for (int32_t i = 0; i < (int32_t)bones.size(); ++i)
        {
            auto& bone = bones[i];
            Matrix4f localMat;
            transutil::NewTRS(localMat, bone.LocalTranslation, bone.LocalRotation, bone.LocalScale);

            if (bone.ParentIndex >= 0)
            {
                bone.BindModelMatrix = bones[bone.ParentIndex].BindModelMatrix * localMat;
            }
            else
            {
                bone.BindModelMatrix = localMat;
            }
            bone.InverseBindMatrix = jmath::Inverse(bone.BindModelMatrix);
        }

        return Skeleton::StaticCreate(name, std::move(bones), skeletonRootIndex);
    }

    static void ProcessNode(
        FbxNode* fbxNode,
        ObjectPtr<Node> parentNode,
        RCPtr<Prefab> pscene,
        FBXImporterSettings* settings,
        bool inverseCoordsystem,
        const string& meshFolder,
        array_list<RCPtr<AssetObject>>& importedAssets,
        RCPtr<Skeleton> skeleton
        )
    {
        auto newNodeName = fbxNode->GetName();
        const auto newNode = pscene->NewNode(newNodeName, parentNode);

        // 骨骼节点从 Skeleton reference 中读取 local TRS，确保 scene node 与 reference skeleton 一致
        if (skeleton && IsSkeletonNode(fbxNode))
        {
            int32_t boneIndex = skeleton->FindBoneIndex(newNodeName);
            if (boneIndex >= 0)
            {
                const auto& bone = skeleton->GetBones()[boneIndex];
                ApplyNodeTransform(newNode.GetPtr(), bone.LocalTranslation, bone.LocalRotation, bone.LocalScale);
            }
            else
            {
                ApplyNodeTransform(fbxNode, newNode.GetPtr());
            }
        }
        else
        {
            ApplyNodeTransform(fbxNode, newNode.GetPtr());
        }

        // 检查 mesh 是否有 skin deformer
        bool hasSkinDeformer = false;
        FbxMesh* skinnedMeshAttr = nullptr;
        for (int i = 0; i < fbxNode->GetNodeAttributeCount(); ++i)
        {
            auto attr = fbxNode->GetNodeAttributeByIndex(i);
            if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
            {
                auto mesh = static_cast<FbxMesh*>(attr);
                bool hasSkin = HasSkinDeformer(mesh);
                if (hasSkin)
                {
                    hasSkinDeformer = true;
                    skinnedMeshAttr = mesh;
                    break;
                }
            }
        }

        // SkinnedMesh 路径
        if (hasSkinDeformer && skeleton && skinnedMeshAttr)
        {
            if (auto skinnedMesh = ProcessSkinnedMesh(fbxNode, skeleton, inverseCoordsystem, settings->RecomputeTangents, settings->UseMikktspace))
            {
                const auto meshPath = meshFolder + "/" + skinnedMesh->GetName();
                AssetDatabase::CreateAsset(skinnedMesh, meshPath);
                auto renderer = newNode->AddComponent<SkinnedMeshRendererComponent>();
                renderer->SetSkinnedMesh(skinnedMesh);
                // root 将在所有节点创建完成后统一绑定
                ApplyMaterialPolicyImpl(skinnedMesh->GetMaterialCount(), skinnedMesh->GetMaterialNames(), settings, meshFolder, importedAssets,
                    [renderer](int idx, RCPtr<Material> mat) { renderer->SetMaterial(idx, mat); });
                importedAssets.push_back(skinnedMesh);
            }
        }
        // StaticMesh 路径
        else if (auto staticMesh = ProcessMesh(fbxNode, inverseCoordsystem, settings->RecomputeTangents, settings->UseMikktspace))
        {
            const auto meshPath = meshFolder + "/" + staticMesh->GetName();
            AssetDatabase::CreateAsset(staticMesh, meshPath);
            auto renderer = newNode->AddComponent<StaticMeshRendererComponent>();
            renderer->SetStaticMesh(staticMesh);
            ApplyMaterialPolicyImpl(staticMesh->GetMaterialCount(), staticMesh->GetMaterialNames(), settings, meshFolder, importedAssets,
                [renderer](int idx, RCPtr<Material> mat) { renderer->SetMaterial(idx, mat); });
            importedAssets.push_back(staticMesh);
        }

        const auto childCount = fbxNode->GetChildCount();
        for (int childIndex = 0; childIndex < childCount; childIndex++)
        {
            const auto childFbxNode = fbxNode->GetChild(childIndex);
            ProcessNode(childFbxNode, newNode, pscene, settings, inverseCoordsystem, meshFolder, importedAssets, skeleton);
        }
    }

    static void LogSkeletonComparison(const Skeleton* preSkeleton, const Skeleton* postSkeleton, const Skeleton* finalSkeleton)
    {
        if (!preSkeleton || !postSkeleton || !finalSkeleton) return;
        const auto& preBones = preSkeleton->GetBones();
        const auto& postBones = postSkeleton->GetBones();
        const auto& finalBones = finalSkeleton->GetBones();

        Logger::Log("========== ConvertScene Skeleton Comparison ==========");
        for (size_t i = 0; i < preBones.size(); ++i)
        {
            const auto& pre = preBones[i];
            const auto& post = postBones[i];
            const auto& fin = finalBones[i];
            Logger::Log(StringUtil::Concat("Bone[", std::to_string(i), "] ", pre.Name));
            Logger::Log(StringUtil::Concat("  Pre-Convert  T=(", std::to_string(pre.LocalTranslation.x), ",", std::to_string(pre.LocalTranslation.y), ",", std::to_string(pre.LocalTranslation.z), ")"
                " R=(", std::to_string(pre.LocalRotation.x), ",", std::to_string(pre.LocalRotation.y), ",", std::to_string(pre.LocalRotation.z), ",", std::to_string(pre.LocalRotation.w), ")"));
            Logger::Log(StringUtil::Concat("  Post-Convert T=(", std::to_string(post.LocalTranslation.x), ",", std::to_string(post.LocalTranslation.y), ",", std::to_string(post.LocalTranslation.z), ")"
                " R=(", std::to_string(post.LocalRotation.x), ",", std::to_string(post.LocalRotation.y), ",", std::to_string(post.LocalRotation.z), ",", std::to_string(post.LocalRotation.w), ")"));
            Logger::Log(StringUtil::Concat("  Final-Fixed  T=(", std::to_string(fin.LocalTranslation.x), ",", std::to_string(fin.LocalTranslation.y), ",", std::to_string(fin.LocalTranslation.z), ")"
                " R=(", std::to_string(fin.LocalRotation.x), ",", std::to_string(fin.LocalRotation.y), ",", std::to_string(fin.LocalRotation.z), ",", std::to_string(fin.LocalRotation.w), ")"));
        }
        Logger::Log("========== End Comparison ==========");
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
            LoadScene(fbxManager, fbxScene, importFile.string().c_str());

            bool inverseCoordSystem = false;

            // 在 ConvertScene 之前提取一次骨骼数据用于对比
            const auto filename = PathUtil::GetFilenameWithoutExt(importFile.filename().string());
            RCPtr<Skeleton> preConvertSkeleton = ProcessSkeleton(fbxScene, filename + "Skeleton_Pre", false);

            if (fbxsetting->ConvertAxisSystem)
            {
                const auto axisSystem = fbxScene->GetGlobalSettings().GetAxisSystem();
                if (axisSystem.GetCoorSystem() == FbxAxisSystem::eRightHanded)
                {
                    inverseCoordSystem = true;
                }
                const auto ourAxisSystem = FbxAxisSystem(FbxAxisSystem::eYAxis, FbxAxisSystem::eParityOdd, FbxAxisSystem::eLeftHanded);
                
                int upSign = 0, frontSign = 0;
                auto upVec = axisSystem.GetUpVector(upSign);
                auto frontVec = axisSystem.GetFrontVector(frontSign);
                auto coordSystem = axisSystem.GetCoorSystem();
                Logger::Log(StringUtil::Concat("FBX Original AxisSystem: Up=", std::to_string((int)upVec), " Sign=", std::to_string(upSign), 
                    " Front=", std::to_string((int)frontVec), " Sign=", std::to_string(frontSign), 
                    " CoordSystem=", std::to_string((int)coordSystem),
                    " (0=RH, 1=LH)"));
                
                if (axisSystem != ourAxisSystem)
                {
                    ourAxisSystem.ConvertScene(fbxScene);
                    Logger::Log("ConvertScene executed.");
                }
                else
                {
                    Logger::Log("AxisSystem already matches, no conversion.");
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
            // Prefab 资产名 = 纯文件名 + "Prefab"（如 TestSkinnedPrefab）
            // Prefab 内根节点名 = 纯文件名（如 TestSkinned），两者不同名
            auto prefab = Prefab::StaticCreate(filename + "Prefab");
            const auto targetMeshFolder = settings->ImportingTargetFolder;
            const auto rootCount = fbxRootNode->GetChildCount();

            // 提取 Skeleton 资产（如果场景中有骨骼）
            RCPtr<Skeleton> skeleton;
            RCPtr<Skeleton> postConvertSkeleton = ProcessSkeleton(fbxScene, filename + "Skeleton_Post", false);
            if (skeleton = ProcessSkeleton(fbxScene, filename + "Skeleton", inverseCoordSystem))
            {
                const auto skeletonPath = targetMeshFolder + "/" + skeleton->GetName();
                AssetDatabase::CreateAsset(skeleton, skeletonPath);
                importedAssets.push_back(skeleton);
            }

            // 输出 ConvertScene 前后对比
            if (preConvertSkeleton && postConvertSkeleton && skeleton)
            {
                LogSkeletonComparison(preConvertSkeleton.GetPtr(), postConvertSkeleton.GetPtr(), skeleton.GetPtr());
            }

            // Prefab 只有一个根节点，名字为纯文件名
            auto prefabRoot = prefab->NewNode(filename, nullptr);
            for (int i = 0; i < rootCount; ++i)
            {
                auto sceneRootNode = fbxRootNode->GetChild(i);
                if (!sceneRootNode)
                    continue;
                ProcessNode(sceneRootNode, prefabRoot, prefab, fbxsetting, inverseCoordSystem, targetMeshFolder, importedAssets, skeleton);
            }

            // 后处理：为所有 SkinnedMeshRendererComponent 绑定骨骼引用
            int skinnedRendererCount = 0;
            for (auto& node : *prefab->GetNodes())
            {
                if (!node) continue;
                if (auto renderer = node->GetComponent<SkinnedMeshRendererComponent>())
                {
                    if (auto sk = renderer->GetSkinnedMesh())
                    {
                        if (auto skel = sk->GetSkeleton())
                        {
                            int rootIdx = skel->GetRootBoneIndex();
                            if (rootIdx >= 0)
                            {
                                const string& rootBoneName = skel->GetBones()[rootIdx].Name;
                                // 在所有节点中查找 skeleton root
                                ObjectPtr<Node> rootBoneNode;
                                for (const auto& n : *prefab->GetNodes())
                                {
                                    if (n->GetName() == rootBoneName)
                                    {
                                        rootBoneNode = n;
                                        break;
                                    }
                                }
                                if (rootBoneNode)
                                {
                                    renderer->SetRoot(rootBoneNode->GetTransform());
                                    skinnedRendererCount++;
                                }
                            }
                        }
                    }
                }
            }
            if (skinnedRendererCount > 0)
            {
                Logger::Log(StringUtil::Concat("Bound ", std::to_string(skinnedRendererCount), " SkinnedMeshRenderer(s) to skeleton root"));
            }

            AssetDatabase::CreateAsset(prefab, settings->ImportingTargetFolder + "/" + filename + "Prefab");
            importedAssets.push_back(prefab);

            DestroySdkObjects(fbxManager, 0);
        }
        return importedAssets;
    }

} // namespace pulsared
