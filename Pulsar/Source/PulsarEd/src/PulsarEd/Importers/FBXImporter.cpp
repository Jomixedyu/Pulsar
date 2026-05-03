#include "Importers/FBXImporter.h"
#include "Importers/FBXImporterInternal.h"

#include "Pulsar/Prefab.h"
#include "Pulsar/Scene.h"

#include <Pulsar/Assets/NodeCollection.h>
#include <Pulsar/Assets/Material.h>
#include <CoreLib/UString.h>
#include <Pulsar/Components/RendererComponent.h>
#include <Pulsar/Components/StaticMeshRendererComponent.h>
#include <Pulsar/Components/SkinnedMeshRendererComponent.h>
#include <Pulsar/TransformUtil.h>
#include <Pulsar/AssetManager.h>
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
        if (hasSkinDeformer && skeleton && skinnedMeshAttr && !settings->SkinnedMeshToStaticMesh)
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
