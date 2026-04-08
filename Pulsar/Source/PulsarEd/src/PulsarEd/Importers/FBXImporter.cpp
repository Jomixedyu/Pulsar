#include "Importers/FBXImporter.h"

#include "Pulsar/Prefab.h"
#include "Pulsar/Scene.h"

#include <Pulsar/Assets/NodeCollection.h>
#include <Pulsar/Assets/StaticMesh.h>
#include <Pulsar/Assets/SkinnedMesh.h>
#include <Pulsar/Assets/AnimationClip.h>
#include <Pulsar/Components/RendererComponent.h>
#include <Pulsar/Components/StaticMeshRendererComponent.h>
#include <Pulsar/Components/SkinnedMeshRendererComponent.h>
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

    // -----------------------------------------------------------------------
    // 辅助：检测某个 FbxMesh 是否含有蒙皮（Skin deformer）
    // -----------------------------------------------------------------------
    static bool MeshHasSkin(FbxMesh* fbxMesh)
    {
        for (int d = 0; d < fbxMesh->GetDeformerCount(FbxDeformer::eSkin); ++d)
        {
            auto* skin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(d, FbxDeformer::eSkin));
            if (skin && skin->GetClusterCount() > 0)
                return true;
        }
        return false;
    }

    // -----------------------------------------------------------------------
    // 辅助：FbxAMatrix → Matrix4f（列主序转置）
    // -----------------------------------------------------------------------
    static Matrix4f ToMatrix4f(const FbxAMatrix& m)
    {
        Matrix4f result;
        for (int r = 0; r < 4; ++r)
            for (int c = 0; c < 4; ++c)
                result[r][c] = (float)m.Get(r, c);
        return result;
    }

    // -----------------------------------------------------------------------
    // 从 FbxSkin 提取完整骨骼层级 + InverseBindMatrix
    // 返回：骨骼列表（按索引）；out_nodeToIndex 供后续蒙皮权重查表
    // -----------------------------------------------------------------------
    static array_list<BoneInfo> ExtractSkeleton(
        FbxSkin* skin,
        std::unordered_map<FbxNode*, int>& out_nodeToIndex)
    {
        array_list<BoneInfo> bones;

        // 先用 cluster 收集所有骨骼节点
        const int clusterCount = skin->GetClusterCount();
        bones.reserve(clusterCount);
        out_nodeToIndex.reserve(clusterCount);

        for (int ci = 0; ci < clusterCount; ++ci)
        {
            FbxCluster* cluster = skin->GetCluster(ci);
            FbxNode*    boneNode = cluster->GetLink();
            if (!boneNode) continue;

            // 避免重复
            if (out_nodeToIndex.count(boneNode)) continue;

            int idx = (int)bones.size();
            out_nodeToIndex[boneNode] = idx;

            BoneInfo bone;
            bone.Name        = boneNode->GetName();
            bone.ParentIndex = -1; // 稍后修正

            // InverseBindMatrix = TransformLinkMatrix^-1 * TransformMatrix
            FbxAMatrix transformMatrix, transformLinkMatrix;
            cluster->GetTransformMatrix(transformMatrix);
            cluster->GetTransformLinkMatrix(transformLinkMatrix);
            FbxAMatrix globalBindposeInverse = transformLinkMatrix.Inverse() * transformMatrix;
            bone.InverseBindMatrix = ToMatrix4f(globalBindposeInverse);

            bones.push_back(std::move(bone));
        }

        // 修正父骨骼索引
        for (auto& [node, idx] : out_nodeToIndex)
        {
            FbxNode* parent = node->GetParent();
            if (parent && out_nodeToIndex.count(parent))
            {
                bones[idx].ParentIndex = out_nodeToIndex[parent];
            }
        }

        return bones;
    }

    // -----------------------------------------------------------------------
    // 从 FbxSkin 提取每控制点的骨骼权重（最多 4 个，归一化）
    // 返回 [controlPoint][4] 的 pair(boneIndex, weight)
    // -----------------------------------------------------------------------
    using BoneInfluence = std::pair<uint32_t, float>;
    static array_list<std::array<BoneInfluence, SKINNEDMESH_MAX_BONE_INFLUENCES>>
    ExtractSkinWeights(FbxSkin* skin, int controlPointCount,
                       const std::unordered_map<FbxNode*, int>& nodeToIndex)
    {
        // [ctrlPt] -> sorted by weight descending
        array_list<std::vector<BoneInfluence>> raw(controlPointCount);

        const int clusterCount = skin->GetClusterCount();
        for (int ci = 0; ci < clusterCount; ++ci)
        {
            FbxCluster* cluster = skin->GetCluster(ci);
            FbxNode*    boneNode = cluster->GetLink();
            if (!boneNode) continue;
            auto it = nodeToIndex.find(boneNode);
            if (it == nodeToIndex.end()) continue;
            int boneIdx = it->second;

            int count = cluster->GetControlPointIndicesCount();
            int* indices = cluster->GetControlPointIndices();
            double* weights = cluster->GetControlPointWeights();

            for (int i = 0; i < count; ++i)
            {
                int cp = indices[i];
                raw[cp].emplace_back((uint32_t)boneIdx, (float)weights[i]);
            }
        }

        // 每控制点：按权重降序，截取前 4 个，归一化
        array_list<std::array<BoneInfluence, SKINNEDMESH_MAX_BONE_INFLUENCES>> result(controlPointCount);
        for (int cp = 0; cp < controlPointCount; ++cp)
        {
            auto& influences = raw[cp];
            std::sort(influences.begin(), influences.end(),
                [](const BoneInfluence& a, const BoneInfluence& b){ return a.second > b.second; });

            float totalWeight = 0.f;
            int   take = std::min((int)influences.size(), SKINNEDMESH_MAX_BONE_INFLUENCES);
            for (int k = 0; k < take; ++k)
                totalWeight += influences[k].second;
            if (totalWeight < 1e-6f) totalWeight = 1.f;

            for (int k = 0; k < SKINNEDMESH_MAX_BONE_INFLUENCES; ++k)
            {
                if (k < take)
                    result[cp][k] = {influences[k].first, influences[k].second / totalWeight};
                else
                    result[cp][k] = {0, 0.f};
            }
        }
        return result;
    }

    // -----------------------------------------------------------------------
    // ProcessSkinnedMesh：提取带蒙皮的网格
    // -----------------------------------------------------------------------
    // SkeletonCache：以根骨骼 FbxNode* 为 Key，整个 FBX 共享同一套骨骼
    struct SkeletonCacheEntry
    {
        RCPtr<Skeleton>                    Skeleton;
        std::unordered_map<FbxNode*, int>  NodeToIndex;  // 动画提取时用
    };
    using SkeletonCache = std::unordered_map<FbxNode*, SkeletonCacheEntry>;

    // -----------------------------------------------------------------------
    // ExtractAnimationClips：从 FbxScene 提取所有 AnimStack，按 30fps 采样
    // -----------------------------------------------------------------------
    static array_list<pulsar::RCPtr<pulsar::AnimationClip>> ExtractAnimationClips(
        FbxScene* scene,
        const SkeletonCacheEntry& entry,
        const string& meshName)
    {
        using namespace pulsar;
        array_list<RCPtr<AnimationClip>> clips;

        const float sampleFps = 30.0f;
        const int stackCount = scene->GetSrcObjectCount<FbxAnimStack>();

        for (int si = 0; si < stackCount; ++si)
        {
            FbxAnimStack* animStack = scene->GetSrcObject<FbxAnimStack>(si);
            scene->SetCurrentAnimationStack(animStack);

            FbxTimeSpan span  = animStack->GetLocalTimeSpan();
            double startSec   = span.GetStart().GetSecondDouble();
            double endSec     = span.GetStop().GetSecondDouble();
            float  duration   = (float)(endSec - startSec);
            if (duration <= 0.f) continue;

            int frameCount = std::max(2, (int)(duration * sampleFps) + 1);

            array_list<BoneAnimTrack> tracks;
            tracks.reserve(entry.NodeToIndex.size());

            for (auto& [boneNode, boneIdx] : entry.NodeToIndex)
            {
                BoneAnimTrack track;
                track.BoneName = boneNode->GetName();
                track.PositionKeys.reserve(frameCount);
                track.RotationKeys.reserve(frameCount);
                track.ScaleKeys.reserve(frameCount);

                for (int fi = 0; fi < frameCount; ++fi)
                {
                    double t = startSec + fi / (double)sampleFps;
                    FbxTime fbxTime;
                    fbxTime.SetSecondDouble(t);

                    FbxAMatrix local = boneNode->EvaluateLocalTransform(fbxTime);

                    float keyTime = (float)(t - startSec);

                    auto T = local.GetT();
                    track.PositionKeys.push_back({keyTime, {(float)T[0], (float)T[1], (float)T[2]}});

                    auto Q = local.GetQ();
                    // Quat4f 构造：(x, y, z, w)
                    track.RotationKeys.push_back({keyTime, {(float)Q[0], (float)Q[1], (float)Q[2], (float)Q[3]}});

                    auto S = local.GetS();
                    track.ScaleKeys.push_back({keyTime, {(float)S[0], (float)S[1], (float)S[2]}});
                }
                tracks.push_back(std::move(track));
            }

            string clipName = meshName + "_" + animStack->GetName();
            auto clip = AnimationClip::StaticCreate(clipName, entry.Skeleton, duration, sampleFps, std::move(tracks));
            clips.push_back(clip);
        }
        return clips;
    }

    // -----------------------------------------------------------------------
    // 返回 {SkinnedMesh, Skeleton}（Skeleton 为独立资产）

    static std::pair<RCPtr<SkinnedMesh>, RCPtr<Skeleton>>
    ProcessSkinnedMesh(FbxNode* fbxNode, bool inverseCoordsystem, SkeletonCache& skeletonCache)
    {
        const auto name = fbxNode->GetName();

        array_list<BoneInfo>           allBones;
        array_list<SkinnedMeshSection> sections;
        array_list<string>             materialNames;

        bool bonesExtracted = false;
        std::unordered_map<FbxNode*, int> nodeToIndex;

        materialNames.reserve(fbxNode->GetMaterialCount());
        for (int i = 0; i < fbxNode->GetMaterialCount(); i++)
            materialNames.push_back(fbxNode->GetMaterial(i)->GetName());

        const auto attrCount = fbxNode->GetNodeAttributeCount();
        for (int attrIndex = 0; attrIndex < attrCount; attrIndex++)
        {
            auto attr = fbxNode->GetNodeAttributeByIndex(attrIndex);
            if (attr->GetAttributeType() != FbxNodeAttribute::eMesh)
                continue;

            auto* fbxMesh = static_cast<FbxMesh*>(attr);

            // 提取骨骼（只做一次）
            FbxSkin* skin = nullptr;
            for (int d = 0; d < fbxMesh->GetDeformerCount(FbxDeformer::eSkin); ++d)
            {
                skin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(d, FbxDeformer::eSkin));
                if (skin) break;
            }

            if (!bonesExtracted && skin)
            {
                allBones      = ExtractSkeleton(skin, nodeToIndex);
                bonesExtracted = true;
            }

            // 每控制点蒙皮权重
            array_list<std::array<BoneInfluence, SKINNEDMESH_MAX_BONE_INFLUENCES>> cpWeights;
            if (skin)
                cpWeights = ExtractSkinWeights(skin, fbxMesh->GetControlPointsCount(), nodeToIndex);

            constexpr int kPolygonSize = 3;
            const auto vertexCount  = fbxMesh->GetPolygonVertexCount();
            const auto polygonCount = fbxMesh->GetPolygonCount();
            assert(vertexCount == polygonCount * kPolygonSize);

            SkinnedMeshSection section;
            const uint8_t numUV = (uint8_t)std::min(fbxMesh->GetUVLayerCount(), (int)STATICMESH_MAX_TEXTURE_COORDS);
            section.NumTexCoords = numUV;
            section.Positions.resize(vertexCount);
            section.Normals.resize(vertexCount);
            section.TexCoords.resize(numUV);
            for (uint8_t uvIdx = 0; uvIdx < numUV; uvIdx++)
                section.TexCoords[uvIdx].resize(vertexCount);
            section.Indices.resize(vertexCount);
            section.BoneIndices.resize(vertexCount);
            section.BoneWeights.resize(vertexCount);

            const bool hasColors   = fbxMesh->GetLayer(0) && fbxMesh->GetLayer(0)->GetVertexColors();
            const bool hasTangents = fbxMesh->GetElementTangentCount() > 0 && fbxMesh->GetElementBinormalCount() > 0;
            if (hasColors)   section.Colors.resize(vertexCount);
            if (hasTangents) section.Tangents.resize(vertexCount);

            for (int polyIndex = 0; polyIndex < polygonCount; polyIndex++)
            {
                for (int vertInFace = 0; vertInFace < kPolygonSize; vertInFace++)
                {
                    const int vertexIndex       = polyIndex * kPolygonSize + vertInFace;
                    const int controlPointIndex = fbxMesh->GetPolygonVertex(polyIndex, vertInFace);

                    section.Positions[vertexIndex] = ToVector3f(fbxMesh->GetControlPointAt(controlPointIndex));

                    FbxVector4 normal;
                    fbxMesh->GetPolygonVertexNormal(polyIndex, vertInFace, normal);
                    section.Normals[vertexIndex] = ToVector3f(normal);

                    if (hasTangents)
                    {
                        const Vector3f T = ToVector3f(GetColorLayerElement(fbxMesh->GetElementTangent(0),  controlPointIndex, vertexIndex));
                        const Vector3f B = ToVector3f(GetColorLayerElement(fbxMesh->GetElementBinormal(0), controlPointIndex, vertexIndex));
                        const Vector3f N = section.Normals[vertexIndex];
                        const float    w = Dot(Cross(N, T), B) > 0.0f ? 1.0f : -1.0f;
                        section.Tangents[vertexIndex] = Vector4f{T.x, T.y, T.z, w};
                    }

                    for (uint8_t uvIdx = 0; uvIdx < numUV; uvIdx++)
                    {
                        auto uv = ToVector2f(GetUVLayerElement(fbxMesh->GetElementUV(uvIdx), controlPointIndex, vertexIndex));
                        if (inverseCoordsystem) uv.y = 1.0f - uv.y;
                        section.TexCoords[uvIdx][vertexIndex] = uv;
                    }

                    if (hasColors)
                    {
                        section.Colors[vertexIndex] = ToColor(GetColorLayerElement(
                            fbxMesh->GetLayer(0)->GetVertexColors(), controlPointIndex, vertexIndex));
                    }

                    // 蒙皮权重（按控制点索引查表）
                    if (!cpWeights.empty())
                    {
                        for (int k = 0; k < SKINNEDMESH_MAX_BONE_INFLUENCES; ++k)
                        {
                            section.BoneIndices[vertexIndex][k] = cpWeights[controlPointIndex][k].first;
                            section.BoneWeights[vertexIndex][k] = cpWeights[controlPointIndex][k].second;
                        }
                    }

                    // 索引（三角形翻转，与 StaticMesh 一致）
                    int indicesValue = vertexIndex;
                    {
                        if (vertInFace == 1) indicesValue += 1;
                        if (vertInFace == 2) indicesValue -= 1;
                    }
                    section.Indices[vertexIndex] = indicesValue;
                }
            }

            section.MaterialIndex = attrIndex;
            sections.push_back(std::move(section));
        }

        if (sections.empty())
            return {nullptr, nullptr};

        // 找根骨骼节点（parentIndex == -1 且 FbxNode 父节点不在 nodeToIndex 里）
        FbxNode* rootBoneNode = nullptr;
        for (auto& [node, idx] : nodeToIndex)
        {
            if (allBones[idx].ParentIndex == -1)
            {
                rootBoneNode = node;
                break;
            }
        }

        // 查缓存，相同根骨骼节点复用同一个 Skeleton 资产
        RCPtr<Skeleton> skeleton;
        if (rootBoneNode && skeletonCache.count(rootBoneNode))
        {
            skeleton = skeletonCache[rootBoneNode].Skeleton;
        }
        else
        {
            string skeletonName = string(name) + "_Skeleton";
            skeleton = Skeleton::StaticCreate(skeletonName, std::move(allBones));
            if (rootBoneNode)
            {
                SkeletonCacheEntry entry;
                entry.Skeleton     = skeleton;
                entry.NodeToIndex  = std::move(nodeToIndex);
                skeletonCache[rootBoneNode] = std::move(entry);
            }
        }

        auto mesh = SkinnedMesh::StaticCreate(name, skeleton, std::move(sections), std::move(materialNames));
        return {mesh, skeleton};
    }

    // -----------------------------------------------------------------------
    // ProcessMesh（StaticMesh，原有逻辑不变）
    // -----------------------------------------------------------------------
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

    // -----------------------------------------------------------------------
    // 辅助：提取节点 TRS 并应用到 transform
    // -----------------------------------------------------------------------
    static void ApplyNodeTransform(FbxNode* fbxNode, Node* node)
    {
        auto translation = ToVector3f(fbxNode->LclTranslation.Get());
        auto scaling     = ToVector3f(fbxNode->LclScaling.Get());

        FbxEuler::EOrder order{};
        fbxNode->GetRotationOrder(FbxNode::EPivotSet::eSourcePivot, order);
        FbxQuaternion q;
        q.ComposeSphericalXYZ(FbxVector4(fbxNode->LclRotation.Get()));
        auto rotation = ToQuat(q);

        auto transform = node->GetTransform();
        transform->SetPosition(translation);
        transform->SetRotation(rotation);
        transform->SetScale(scaling);
    }

    static void ProcessNode(
        FbxNode* fbxNode,
        ObjectPtr<Node> parentNode,
        RCPtr<Prefab> pscene,
        FBXImporterSettings* settings,
        bool inverseCoordsystem,
        const string& meshFolder,
        array_list<RCPtr<AssetObject>>& importedAssets,
        SkeletonCache& skeletonCache
        )
    {
        auto newNodeName = fbxNode->GetName();
        const auto newNode = pscene->NewNode(newNodeName, parentNode);

        // 检测第一个 eMesh attribute 是否有 Skin
        bool hasSkin = false;
        for (int attrIndex = 0; attrIndex < fbxNode->GetNodeAttributeCount(); attrIndex++)
        {
            auto attr = fbxNode->GetNodeAttributeByIndex(attrIndex);
            if (attr->GetAttributeType() == FbxNodeAttribute::eMesh)
            {
                hasSkin = MeshHasSkin(static_cast<FbxMesh*>(attr));
                break;
            }
        }

        if (hasSkin)
        {
            // SkinnedMesh 路径（传入 skeletonCache，相同骨骼复用同一 Skeleton 资产）
            auto [skinnedMesh, skeleton] = ProcessSkinnedMesh(fbxNode, inverseCoordsystem, skeletonCache);
            if (skinnedMesh)
            {
                // Skeleton：只在第一次出现时（不在 importedAssets 里）才写入资产文件
                bool skeletonAlreadySaved = false;
                for (auto& a : importedAssets)
                    if (a.GetPtr() == skeleton.GetPtr()) { skeletonAlreadySaved = true; break; }

                if (!skeletonAlreadySaved)
                {
                    const auto skeletonPath = meshFolder + "/" + skeleton->GetName();
                    AssetDatabase::CreateAsset(skeleton, skeletonPath);
                    importedAssets.push_back(skeleton);
                }

                // SkinnedMesh 每次都保存
                const auto meshPath = meshFolder + "/" + skinnedMesh->GetName();
                AssetDatabase::CreateAsset(skinnedMesh, meshPath);
                importedAssets.push_back(skinnedMesh);

                auto renderer = newNode->AddComponent<SkinnedMeshRendererComponent>();
                renderer->SetSkinnedMesh(skinnedMesh);
                ApplyNodeTransform(fbxNode, newNode.GetPtr());
            }
        }
        else
        {
            // StaticMesh 路径（原有逻辑）
            if (auto staticMesh = ProcessMesh(fbxNode, inverseCoordsystem))
            {
                const auto meshPath = meshFolder + "/" + staticMesh->GetName();
                AssetDatabase::CreateAsset(staticMesh, meshPath);
                newNode->AddComponent<StaticMeshRendererComponent>()->SetStaticMesh(staticMesh);
                ApplyNodeTransform(fbxNode, newNode.GetPtr());
                importedAssets.push_back(staticMesh);
            }
        }

        const auto childCount = fbxNode->GetChildCount();
        for (int childIndex = 0; childIndex < childCount; childIndex++)
        {
            const auto childFbxNode = fbxNode->GetChild(childIndex);
            ProcessNode(childFbxNode, newNode, pscene, settings, inverseCoordsystem, meshFolder, importedAssets, skeletonCache);
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
            LoadScene(fbxManager, fbxScene, importFile.string().c_str());

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
            const auto filename = PathUtil::GetFilenameWithoutExt(importFile.filename().string());
            auto prefab = Prefab::StaticCreate(filename);
            const auto targetMeshFolder = settings->ImportingTargetFolder + "/" + filename + "_Items";
            const auto rootCount = fbxRootNode->GetChildCount();
            SkeletonCache skeletonCache; // 整个 FBX 文件共享同一套骨骼缓存
            for (int i = 0; i < rootCount; ++i)
            {
                auto sceneRootNode = fbxRootNode->GetChild(i);
                ProcessNode(sceneRootNode, nullptr, prefab, fbxsetting, inverseCoordSystem, targetMeshFolder, importedAssets, skeletonCache);
            }

            // 动画提取：每个 Skeleton 只提取一次
            for (auto& [rootBone, entry] : skeletonCache)
            {
                string skeletonName = entry.Skeleton->GetName();
                // 去掉 "_Skeleton" 后缀作为 clip 的前缀
                string baseName = skeletonName;
                const string suffix = "_Skeleton";
                if (baseName.size() > suffix.size() &&
                    baseName.substr(baseName.size() - suffix.size()) == suffix)
                    baseName = baseName.substr(0, baseName.size() - suffix.size());

                auto clips = ExtractAnimationClips(fbxScene, entry, baseName);
                for (auto& clip : clips)
                {
                    const auto clipPath = targetMeshFolder + "/" + clip->GetName();
                    AssetDatabase::CreateAsset(clip, clipPath);
                    importedAssets.push_back(pulsar::RCPtr<pulsar::AssetObject>(clip));
                }
            }

            AssetDatabase::CreateAsset(prefab, settings->ImportingTargetFolder + "/" + filename);
            importedAssets.push_back(prefab);

            DestroySdkObjects(fbxManager, 0);
        }
        return importedAssets;
    }

} // namespace pulsared
