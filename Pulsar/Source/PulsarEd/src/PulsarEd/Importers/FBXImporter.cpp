#include "Importers/FBXImporter.h"

#include "Pulsar/Prefab.h"
#include "Pulsar/Scene.h"

#include <Pulsar/Assets/NodeCollection.h>
#include <Pulsar/Assets/StaticMesh.h>
#include <Pulsar/Assets/SkinnedMesh.h>
#include <Pulsar/Assets/AnimationClip.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Components/RendererComponent.h>
#include <Pulsar/Components/StaticMeshRendererComponent.h>
#include <Pulsar/Components/SkinnedMeshRendererComponent.h>
#include <Pulsar/Components/AnimatorComponent.h>
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
    static void LogNodeRotationDebug(FbxNode* fbxNode, const char* phase)
    {
        if (!fbxNode)
            return;

        bool hasSkeletonAttr = false;
        bool hasMeshAttr = false;
        for (int attrIndex = 0; attrIndex < fbxNode->GetNodeAttributeCount(); ++attrIndex)
        {
            auto attr = fbxNode->GetNodeAttributeByIndex(attrIndex);
            if (!attr) continue;
            hasSkeletonAttr = hasSkeletonAttr || attr->GetAttributeType() == FbxNodeAttribute::eSkeleton;
            hasMeshAttr = hasMeshAttr || attr->GetAttributeType() == FbxNodeAttribute::eMesh;
        }

        if (!hasSkeletonAttr && !hasMeshAttr)
            return;

        const auto lclR = fbxNode->LclRotation.Get();
        const auto preR = fbxNode->GetPreRotation(FbxNode::eSourcePivot);
        const auto postR = fbxNode->GetPostRotation(FbxNode::eSourcePivot);
        const auto rotOffset = fbxNode->GetRotationOffset(FbxNode::eSourcePivot);
        const auto rotPivot = fbxNode->GetRotationPivot(FbxNode::eSourcePivot);
        const auto geoR = fbxNode->GetGeometricRotation(FbxNode::eSourcePivot);
        const auto geoT = fbxNode->GetGeometricTranslation(FbxNode::eSourcePivot);
        const auto geoS = fbxNode->GetGeometricScaling(FbxNode::eSourcePivot);
        const auto localEval = fbxNode->EvaluateLocalTransform(FBXSDK_TIME_ZERO);
        const auto globalEval = fbxNode->EvaluateGlobalTransform(FBXSDK_TIME_ZERO);
        const auto localEvalQ = localEval.GetQ();
        const auto globalEvalQ = globalEval.GetQ();
        const auto localEvalT = localEval.GetT();
        const auto globalEvalT = globalEval.GetT();

        Logger::Log(
            string("FBXImporter node rot debug[") + phase + "] - name='" + fbxNode->GetName() +
            "', kind='" + (hasSkeletonAttr ? string("Skeleton") : string("Mesh")) +
            "', lclR=(" + std::to_string((float)lclR[0]) + "," + std::to_string((float)lclR[1]) + "," + std::to_string((float)lclR[2]) + ")" +
            ", preR=(" + std::to_string((float)preR[0]) + "," + std::to_string((float)preR[1]) + "," + std::to_string((float)preR[2]) + ")" +
            ", postR=(" + std::to_string((float)postR[0]) + "," + std::to_string((float)postR[1]) + "," + std::to_string((float)postR[2]) + ")" +
            ", rotOffset=(" + std::to_string((float)rotOffset[0]) + "," + std::to_string((float)rotOffset[1]) + "," + std::to_string((float)rotOffset[2]) + ")" +
            ", rotPivot=(" + std::to_string((float)rotPivot[0]) + "," + std::to_string((float)rotPivot[1]) + "," + std::to_string((float)rotPivot[2]) + ")" +
            ", geoR=(" + std::to_string((float)geoR[0]) + "," + std::to_string((float)geoR[1]) + "," + std::to_string((float)geoR[2]) + ")" +
            ", geoT=(" + std::to_string((float)geoT[0]) + "," + std::to_string((float)geoT[1]) + "," + std::to_string((float)geoT[2]) + ")" +
            ", geoS=(" + std::to_string((float)geoS[0]) + "," + std::to_string((float)geoS[1]) + "," + std::to_string((float)geoS[2]) + ")" +
            ", localEvalT=(" + std::to_string((float)localEvalT[0]) + "," + std::to_string((float)localEvalT[1]) + "," + std::to_string((float)localEvalT[2]) + ")" +
            ", globalEvalT=(" + std::to_string((float)globalEvalT[0]) + "," + std::to_string((float)globalEvalT[1]) + "," + std::to_string((float)globalEvalT[2]) + ")" +
            ", localEvalQ=(" + std::to_string((float)localEvalQ[0]) + "," + std::to_string((float)localEvalQ[1]) + "," + std::to_string((float)localEvalQ[2]) + "," + std::to_string((float)localEvalQ[3]) + ")" +
            ", globalEvalQ=(" + std::to_string((float)globalEvalQ[0]) + "," + std::to_string((float)globalEvalQ[1]) + "," + std::to_string((float)globalEvalQ[2]) + "," + std::to_string((float)globalEvalQ[3]) + ")",
            LogLevel::Warning);
    }

    static void LogNodeRotationDebugRecursive(FbxNode* fbxNode, const char* phase)
    {
        if (!fbxNode)
            return;
        LogNodeRotationDebug(fbxNode, phase);
        for (int i = 0; i < fbxNode->GetChildCount(); ++i)
        {
            LogNodeRotationDebugRecursive(fbxNode->GetChild(i), phase);
        }
    }

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

    static Quat4f MatrixToQuat(const Matrix4f& m)
    {
        float fourXSquaredMinus1 = m[0][0] - m[1][1] - m[2][2];
        float fourYSquaredMinus1 = m[1][1] - m[0][0] - m[2][2];
        float fourZSquaredMinus1 = m[2][2] - m[0][0] - m[1][1];
        float fourWSquaredMinus1 = m[0][0] + m[1][1] + m[2][2];

        int biggestIndex = 0;
        float fourBiggestSquaredMinus1 = fourWSquaredMinus1;
        if (fourXSquaredMinus1 > fourBiggestSquaredMinus1)
        {
            fourBiggestSquaredMinus1 = fourXSquaredMinus1;
            biggestIndex = 1;
        }
        if (fourYSquaredMinus1 > fourBiggestSquaredMinus1)
        {
            fourBiggestSquaredMinus1 = fourYSquaredMinus1;
            biggestIndex = 2;
        }
        if (fourZSquaredMinus1 > fourBiggestSquaredMinus1)
        {
            fourBiggestSquaredMinus1 = fourZSquaredMinus1;
            biggestIndex = 3;
        }

        float biggestVal = sqrtf(fourBiggestSquaredMinus1 + 1.f) * 0.5f;
        float mult = 0.25f / biggestVal;

        switch (biggestIndex)
        {
        case 0:
            return {(m[1][2] - m[2][1]) * mult, (m[2][0] - m[0][2]) * mult, (m[0][1] - m[1][0]) * mult, biggestVal};
        case 1:
            return {biggestVal, (m[0][1] + m[1][0]) * mult, (m[2][0] + m[0][2]) * mult, (m[1][2] - m[2][1]) * mult};
        case 2:
            return {(m[0][1] + m[1][0]) * mult, biggestVal, (m[1][2] + m[2][1]) * mult, (m[2][0] - m[0][2]) * mult};
        case 3:
            return {(m[2][0] + m[0][2]) * mult, (m[1][2] + m[2][1]) * mult, biggestVal, (m[0][1] - m[1][0]) * mult};
        default:
            assert(false);
            return {0.f, 0.f, 0.f, 1.f};
        }
    }

    static inline FbxAMatrix EvaluateReconstructedLocalTransform(FbxNode* node, const FbxTime& time)
    {
        FbxAMatrix global = node->EvaluateGlobalTransform(time);
        FbxNode* parent = node->GetParent();
        return parent ? parent->EvaluateGlobalTransform(time).Inverse() * global : global;
    }

    struct BoneRebuildInfo
    {
        FbxNode* Node = nullptr;
        int Index = -1;
        int ParentIndex = -1;

        // Joint node local TRS: scene node / animation 均应消费这组数据。
        Vector3f LocalTranslation = {0.f, 0.f, 0.f};
        Vector3f LocalScale = {1.f, 1.f, 1.f};
        Quat4f LocalRotation = {0.f, 0.f, 0.f, 1.f};
        Matrix4f LocalMatrix = Matrix4f(1.f);
        Matrix4f WorldMatrix = Matrix4f(1.f);

        // Bone segment bind space: skeleton / skinning 仅应消费这组数据。
        Matrix4f SegmentWorldMatrix = Matrix4f(1.f);
        Matrix4f ModelMatrix = Matrix4f(1.f);
        Matrix4f InverseBindMatrix = Matrix4f(1.f);

        // Source FBX local/world TRS：仅用于单位吸收与调试，不再直接暴露给场景层。
        Vector3f SourceLocalTranslation = {0.f, 0.f, 0.f};
        Vector3f SourceLocalScale = {1.f, 1.f, 1.f};
        Matrix4f SourceLocalMatrix = Matrix4f(1.f);
        Matrix4f SourceWorldMatrix = Matrix4f(1.f);
    };

    static bool IsSkeletonNode(FbxNode* fbxNode)
    {
        if (!fbxNode)
            return false;
        for (int i = 0; i < fbxNode->GetNodeAttributeCount(); ++i)
        {
            auto* attr = fbxNode->GetNodeAttributeByIndex(i);
            if (attr && attr->GetAttributeType() == FbxNodeAttribute::eSkeleton)
                return true;
        }
        return false;
    }

    static Vector3f ChooseBoneUpReference(const Vector3f& forward)
    {
        const Vector3f worldUp{0.f, 1.f, 0.f};
        const Vector3f worldRight{1.f, 0.f, 0.f};
        return std::abs(Dot(forward, worldUp)) < 0.95f ? worldUp : worldRight;
    }

    static Matrix4f BuildBasisFromForward(const Vector3f& desiredForward, const Vector3f* preferredUp = nullptr)
    {
        Vector3f forward = desiredForward;
        if (Dot(forward, forward) <= 1e-8f)
            forward = {1.f, 0.f, 0.f};
        else
            forward = Normalize(forward);

        Vector3f upRef = preferredUp ? *preferredUp : ChooseBoneUpReference(forward);
        if (Dot(upRef, upRef) <= 1e-8f || std::abs(Dot(Normalize(upRef), forward)) > 0.95f)
            upRef = ChooseBoneUpReference(forward);

        Vector3f zAxis = Cross(forward, upRef);
        if (Dot(zAxis, zAxis) < 1e-8f)
        {
            upRef = {0.f, 0.f, 1.f};
            zAxis = Cross(forward, upRef);
        }
        zAxis = Normalize(zAxis);
        Vector3f yAxis = Normalize(Cross(zAxis, forward));

        Matrix4f basis{1.f};
        basis[0][0] = forward.x;
        basis[0][1] = forward.y;
        basis[0][2] = forward.z;
        basis[1][0] = yAxis.x;
        basis[1][1] = yAxis.y;
        basis[1][2] = yAxis.z;
        basis[2][0] = zAxis.x;
        basis[2][1] = zAxis.y;
        basis[2][2] = zAxis.z;
        return basis;
    }

    static Vector3f ComputePulsarBoneForward(FbxNode* boneNode, const FbxTime& time = FBXSDK_TIME_ZERO)
    {
        if (!boneNode)
            return {1.f, 0.f, 0.f};

        FbxAMatrix boneGlobal = boneNode->EvaluateGlobalTransform(time);
        Vector3f bonePos = ToVector3f(boneGlobal.GetT());

        for (int childIndex = 0; childIndex < boneNode->GetChildCount(); ++childIndex)
        {
            FbxNode* child = boneNode->GetChild(childIndex);
            if (!child || !IsSkeletonNode(child))
                continue;

            Vector3f childPos = ToVector3f(child->EvaluateGlobalTransform(time).GetT());
            Vector3f toChild = childPos - bonePos;
            if (Dot(toChild, toChild) > 1e-8f)
                return Normalize(toChild);
        }

        FbxNode* parent = boneNode->GetParent();
        if (parent && IsSkeletonNode(parent))
        {
            Vector3f parentPos = ToVector3f(parent->EvaluateGlobalTransform(time).GetT());
            Vector3f fromParent = bonePos - parentPos;
            if (Dot(fromParent, fromParent) > 1e-8f)
                return Normalize(fromParent);
        }

        auto local = EvaluateReconstructedLocalTransform(boneNode, time);
        Vector3f localT = ToVector3f(local.GetT());
        if (Dot(localT, localT) > 1e-8f)
            return Normalize(localT);
        return {1.f, 0.f, 0.f};
    }

    static BoneRebuildInfo BuildBoneRebuildInfo(FbxNode* boneNode, int boneIndex, int parentIndex, const Matrix4f* parentWorld)
    {
        BoneRebuildInfo info;
        info.Node = boneNode;
        info.Index = boneIndex;
        info.ParentIndex = parentIndex;

        const FbxAMatrix global = boneNode->EvaluateGlobalTransform(FBXSDK_TIME_ZERO);
        const FbxAMatrix local = EvaluateReconstructedLocalTransform(boneNode, FBXSDK_TIME_ZERO);
        const Vector3f worldPosition = ToVector3f(global.GetT());
        const Vector3f localTranslation = ToVector3f(local.GetT());
        const Vector3f localScale = ToVector3f(local.GetS());
        const Quat4f sourceLocalRotation = ToQuat(local.GetQ());

        info.SourceLocalTranslation = localTranslation;
        info.SourceLocalScale = localScale;
        transutil::NewTRS(info.SourceLocalMatrix, localTranslation, sourceLocalRotation, localScale);
        info.SourceWorldMatrix = parentWorld ? (*parentWorld * info.SourceLocalMatrix) : info.SourceLocalMatrix;
        info.SourceWorldMatrix[3][0] = worldPosition.x;
        info.SourceWorldMatrix[3][1] = worldPosition.y;
        info.SourceWorldMatrix[3][2] = worldPosition.z;

        info.LocalScale = localScale;
        info.LocalTranslation = localTranslation;

        const Vector3f targetWorldForward = -ComputePulsarBoneForward(boneNode);
        Matrix4f worldBasis = BuildBasisFromForward(targetWorldForward);
        worldBasis[3][0] = 0.f;
        worldBasis[3][1] = 0.f;
        worldBasis[3][2] = 0.f;
        worldBasis[3][3] = 1.f;

        if (boneNode->GetParent() && IsSkeletonNode(boneNode->GetParent()) && parentWorld)
        {
            Matrix4f parentRotOnly = *parentWorld;
            parentRotOnly[3][0] = 0.f;
            parentRotOnly[3][1] = 0.f;
            parentRotOnly[3][2] = 0.f;
            parentRotOnly[3][3] = 1.f;

            Matrix4f localBasis = jmath::Inverse(parentRotOnly) * worldBasis;
            localBasis[3][0] = 0.f;
            localBasis[3][1] = 0.f;
            localBasis[3][2] = 0.f;
            localBasis[3][3] = 1.f;
            info.LocalRotation = MatrixToQuat(localBasis);
            {
                float qlen = std::sqrt(
                    info.LocalRotation.x * info.LocalRotation.x +
                    info.LocalRotation.y * info.LocalRotation.y +
                    info.LocalRotation.z * info.LocalRotation.z +
                    info.LocalRotation.w * info.LocalRotation.w);
                if (qlen > 1e-8f)
                {
                    info.LocalRotation.x /= qlen;
                    info.LocalRotation.y /= qlen;
                    info.LocalRotation.z /= qlen;
                    info.LocalRotation.w /= qlen;
                }
            }
        }
        else
        {
            info.LocalRotation = MatrixToQuat(worldBasis);
            {
                float qlen = std::sqrt(
                    info.LocalRotation.x * info.LocalRotation.x +
                    info.LocalRotation.y * info.LocalRotation.y +
                    info.LocalRotation.z * info.LocalRotation.z +
                    info.LocalRotation.w * info.LocalRotation.w);
                if (qlen > 1e-8f)
                {
                    info.LocalRotation.x /= qlen;
                    info.LocalRotation.y /= qlen;
                    info.LocalRotation.z /= qlen;
                    info.LocalRotation.w /= qlen;
                }
            }
        }

        transutil::NewTRS(info.LocalMatrix, localTranslation, info.LocalRotation, localScale);
        info.WorldMatrix = parentWorld ? (*parentWorld * info.LocalMatrix) : info.LocalMatrix;
        info.WorldMatrix[3][0] = worldPosition.x;
        info.WorldMatrix[3][1] = worldPosition.y;
        info.WorldMatrix[3][2] = worldPosition.z;

        return info;
    }

    static Matrix4f ToMatrix4f(const FbxAMatrix& m)
    {
        Matrix4f result;
        for (int row = 0; row < 4; ++row)
        {
            for (int col = 0; col < 4; ++col)
            {
                result[col][row] = (float)m.Get(col, row);
            }
        }
        return result;
    }

    static Matrix4f BuildSegmentWorldMatrix(FbxNode* boneNode, const Matrix4f* parentSegmentWorld, const FbxTime& time = FBXSDK_TIME_ZERO)
    {
        FbxAMatrix boneGlobal = boneNode->EvaluateGlobalTransform(time);
        Matrix4f segmentWorld = ToMatrix4f(boneGlobal);

        Vector3f bonePos = ToVector3f(boneGlobal.GetT());
        segmentWorld[3][0] = bonePos.x;
        segmentWorld[3][1] = bonePos.y;
        segmentWorld[3][2] = bonePos.z;
        return segmentWorld;
    }

    static Matrix4f BuildAnimatedWorldMatrix(
        const BoneRebuildInfo& bindInfo,
        const std::vector<Matrix4f>& frameWorlds,
        const FbxTime& time)
    {
        FbxNode* boneNode = bindInfo.Node;
        if (!boneNode)
            return Matrix4f(1.f);

        const FbxAMatrix local = EvaluateReconstructedLocalTransform(boneNode, time);
        const Vector3f localTranslation = ToVector3f(local.GetT());
        const Vector3f localScale = ToVector3f(local.GetS());
        Quat4f localRotation = ToQuat(local.GetQ());
        Matrix4f localMatrix{1.f};
        transutil::NewTRS(localMatrix, localTranslation, localRotation, localScale);

        if (bindInfo.ParentIndex < 0)
            return localMatrix;

        return frameWorlds[bindInfo.ParentIndex] * localMatrix;
    }

    static std::unordered_map<FbxNode*, BoneRebuildInfo> ReconstructPulsarSkeletalHierarchy(
        const std::unordered_map<FbxNode*, int>& nodeToIndex,
        const array_list<BoneInfo>& bones,
        const Matrix4f* modelRootWorld = nullptr)
    {
        std::unordered_map<FbxNode*, BoneRebuildInfo> rebuilt;
        rebuilt.reserve(nodeToIndex.size());

        array_list<std::pair<FbxNode*, int>> orderedNodes;
        orderedNodes.reserve(nodeToIndex.size());
        for (const auto& [node, idx] : nodeToIndex)
            orderedNodes.emplace_back(node, idx);
        std::sort(orderedNodes.begin(), orderedNodes.end(), [](const auto& a, const auto& b) { return a.second < b.second; });

        for (const auto& [node, idx] : orderedNodes)
        {
            const int parentIndex = bones[idx].ParentIndex;
            const Matrix4f* parentWorld = nullptr;
            if (parentIndex >= 0)
            {
                for (const auto& [parentNode, parentInfo] : rebuilt)
                {
                    if (parentInfo.Index == parentIndex)
                    {
                        parentWorld = &parentInfo.WorldMatrix;
                        break;
                    }
                }
            }

            auto info = BuildBoneRebuildInfo(node, idx, parentIndex, parentWorld);
            rebuilt[node] = info;
        }

        if (!orderedNodes.empty())
        {
            auto rootIt = rebuilt.find(orderedNodes.front().first);
            if (rootIt != rebuilt.end())
            {
                for (const auto& [node, idx] : orderedNodes)
                {
                    auto infoIt = rebuilt.find(node);
                    if (infoIt == rebuilt.end())
                        continue;

                    BoneRebuildInfo& info = infoIt->second;
                    const Matrix4f* parentSegmentWorld = nullptr;
                    if (info.ParentIndex >= 0)
                    {
                        for (auto& [parentNode, parentInfo] : rebuilt)
                        {
                            if (parentInfo.Index == info.ParentIndex)
                            {
                                parentSegmentWorld = &parentInfo.SegmentWorldMatrix;
                                break;
                            }
                        }
                    }

                    info.SegmentWorldMatrix = BuildSegmentWorldMatrix(info.Node, parentSegmentWorld);
                }

                Matrix4f modelRootInv = modelRootWorld ? jmath::Inverse(*modelRootWorld) : jmath::Inverse(rootIt->second.SegmentWorldMatrix);
                for (auto& [node, info] : rebuilt)
                {
                    info.ModelMatrix = modelRootInv * info.SegmentWorldMatrix;
                    info.InverseBindMatrix = jmath::Inverse(info.ModelMatrix);
                }
            }
        }

        return rebuilt;
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
    // 从 FbxSkin 提取完整骨骼层级 + InverseBindMatrix
    // 返回：骨骼列表（按索引）；out_nodeToIndex 供后续蒙皮权重查表
    // -----------------------------------------------------------------------
    static string BuildBonePath(FbxNode* boneNode, const std::unordered_map<FbxNode*, int>& nodeToIndex)
    {
        array_list<string> parts;
        for (FbxNode* n = boneNode; n; n = n->GetParent())
        {
            parts.push_back(n->GetName());
            auto* parent = n->GetParent();
            if (!parent || !nodeToIndex.count(parent))
                break;
        }
        std::reverse(parts.begin(), parts.end());

        string path;
        for (size_t i = 0; i < parts.size(); ++i)
        {
            if (i > 0) path += "/";
            path += parts[i];
        }
        return path;
    }

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
            bone.Name = boneNode->GetName();
            bone.ParentIndex = -1; // 稍后修正
            bones.push_back(std::move(bone));
        }

        // 修正父骨骼索引 + 构建层级路径
        for (auto& [node, idx] : out_nodeToIndex)
        {
            FbxNode* parent = node->GetParent();
            if (parent && out_nodeToIndex.count(parent))
            {
                bones[idx].ParentIndex = out_nodeToIndex[parent];
            }
            bones[idx].Path = BuildBonePath(node, out_nodeToIndex);
        }

        // 统一到 skeleton root 的模型空间：
        // runtime 使用 rootCurrent^-1 * boneCurrent，因此 bind pose 也需要使用
        // (rootBind^-1 * boneBind)^-1 作为 InverseBindMatrix。
        int rootIndex = -1;
        FbxNode* rootBoneNode = nullptr;
        for (auto& [node, idx] : out_nodeToIndex)
        {
            if (bones[idx].ParentIndex == -1)
            {
                rootIndex = idx;
                rootBoneNode = node;
                break;
            }
        }

        if (rootBoneNode)
        {
            auto rebuilt = ReconstructPulsarSkeletalHierarchy(out_nodeToIndex, bones);
            int loggedBindPose = 0;

            for (auto& [node, idx] : out_nodeToIndex)
            {
                auto rebuildIt = rebuilt.find(node);
                if (rebuildIt == rebuilt.end())
                    continue;

                bones[idx].LocalTranslation = rebuildIt->second.LocalTranslation;
                bones[idx].LocalRotation = rebuildIt->second.LocalRotation;
                bones[idx].LocalScale = rebuildIt->second.LocalScale;
                bones[idx].BindModelMatrix = rebuildIt->second.ModelMatrix;
                bones[idx].InverseBindMatrix = rebuildIt->second.InverseBindMatrix;

                if (loggedBindPose < 4)
                {
                    const auto& info = rebuildIt->second;
                    auto normalizedQ = info.LocalRotation;
                    Logger::Log(
                        string("FBXImporter rebuilt bind pose debug - bone=") + node->GetName() +
                        ", localT=(" + std::to_string(info.LocalTranslation.x) + "," + std::to_string(info.LocalTranslation.y) + "," + std::to_string(info.LocalTranslation.z) + ")" +
                        ", localQ=(" + std::to_string(normalizedQ.x) + "," + std::to_string(normalizedQ.y) + "," + std::to_string(normalizedQ.z) + "," + std::to_string(normalizedQ.w) + ")" +
                        ", modelPos=(" + std::to_string(info.ModelMatrix[3][0]) + "," + std::to_string(info.ModelMatrix[3][1]) + "," + std::to_string(info.ModelMatrix[3][2]) + ")",
                        LogLevel::Warning);
                    ++loggedBindPose;
                }
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
        std::unordered_map<FbxNode*, BoneRebuildInfo> RebuiltBones;
        string                             RootPath;     // 兼容旧逻辑保留
    };
    using SkeletonCache = std::unordered_map<FbxNode*, SkeletonCacheEntry>;

    struct PendingRootBoneBinding
    {
        ObjectPtr<Node>                    MeshNode;
        SceneObjectPtr<SkinnedMeshRendererComponent> Renderer;
        RCPtr<Skeleton>                    Skeleton;
    };
    using PendingRootBoneBindings = array_list<PendingRootBoneBinding>;

    using RebuiltBoneLookup = std::unordered_map<FbxNode*, const BoneRebuildInfo*>;

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

        array_list<std::pair<FbxNode*, int>> orderedNodes;
        orderedNodes.reserve(entry.NodeToIndex.size());
        for (const auto& [boneNode, boneIdx] : entry.NodeToIndex)
            orderedNodes.emplace_back(boneNode, boneIdx);
        std::sort(orderedNodes.begin(), orderedNodes.end(), [](const auto& a, const auto& b) { return a.second < b.second; });

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

            array_list<const BoneRebuildInfo*> orderedBindInfos;
            orderedBindInfos.reserve(orderedNodes.size());
            for (const auto& [boneNode, boneIdx] : orderedNodes)
            {
                auto rebuildIt = entry.RebuiltBones.find(boneNode);
                if (rebuildIt != entry.RebuiltBones.end())
                {
                    orderedBindInfos.push_back(&rebuildIt->second);
                }
            }

            for (const auto* bindInfoPtr : orderedBindInfos)
            {
                const BoneRebuildInfo& bindInfo = *bindInfoPtr;
                FbxNode* boneNode = bindInfo.Node;
                if (!boneNode)
                    continue;

                BoneAnimTrack track;
                track.BoneName = boneNode->GetName();
                track.PositionKeys.reserve(frameCount);
                track.RotationKeys.reserve(frameCount);
                track.ScaleKeys.reserve(frameCount);
                tracks.push_back(std::move(track));
            }

            for (int fi = 0; fi < frameCount; ++fi)
            {
                double t = startSec + fi / (double)sampleFps;
                FbxTime fbxTime;
                fbxTime.SetSecondDouble(t);

                std::vector<Matrix4f> frameWorlds(orderedBindInfos.size(), Matrix4f(1.f));
                for (size_t boneOrder = 0; boneOrder < orderedBindInfos.size(); ++boneOrder)
                {
                    const BoneRebuildInfo& frameBindInfo = *orderedBindInfos[boneOrder];
                    frameWorlds[boneOrder] = BuildAnimatedWorldMatrix(frameBindInfo, frameWorlds, fbxTime);
                }

                float keyTime = (float)(t - startSec);
                for (size_t boneOrder = 0; boneOrder < orderedBindInfos.size(); ++boneOrder)
                {
                    const BoneRebuildInfo& bindInfo = *orderedBindInfos[boneOrder];
                    Matrix4f animWorld = frameWorlds[boneOrder];
                    Matrix4f animLocal = Matrix4f(1.f);
                    if (bindInfo.ParentIndex >= 0)
                    {
                        animLocal = jmath::Inverse(frameWorlds[bindInfo.ParentIndex]) * animWorld;
                    }
                    else
                    {
                        animLocal = animWorld;
                    }

                    Matrix4f deltaLocal = jmath::Inverse(bindInfo.LocalMatrix) * animLocal;
                    Matrix4f finalLocal = bindInfo.LocalMatrix * deltaLocal;
                    tracks[boneOrder].PositionKeys.push_back({keyTime, {finalLocal[3][0], finalLocal[3][1], finalLocal[3][2]}});
                    tracks[boneOrder].RotationKeys.push_back({keyTime, MatrixToQuat(finalLocal)});
                    tracks[boneOrder].ScaleKeys.push_back({keyTime, bindInfo.LocalScale});
                }
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
            ProcessSkinnedMesh(FbxNode* fbxNode, bool inverseCoordsystem, SkeletonCache& skeletonCache, bool recomputeTangents, bool useMikktspace)
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

            FbxAMatrix meshNodeGlobal = fbxNode->EvaluateGlobalTransform(FBXSDK_TIME_ZERO);
            FbxAMatrix geometryTransform;
            geometryTransform.SetIdentity();
            geometryTransform.SetT(fbxNode->GetGeometricTranslation(FbxNode::eSourcePivot));
            geometryTransform.SetR(fbxNode->GetGeometricRotation(FbxNode::eSourcePivot));
            geometryTransform.SetS(fbxNode->GetGeometricScaling(FbxNode::eSourcePivot));

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

            FbxAMatrix modelRootGlobal = meshNodeGlobal;
            FbxAMatrix modelRootGlobalInv = modelRootGlobal.Inverse();

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
            if (hasColors) section.Colors.resize(vertexCount);
            if (recomputeTangents || hasTangents) section.Tangents.resize(vertexCount);

            for (int polyIndex = 0; polyIndex < polygonCount; polyIndex++)
            {
                for (int vertInFace = 0; vertInFace < kPolygonSize; vertInFace++)
                {
                    const int vertexIndex       = polyIndex * kPolygonSize + vertInFace;
                    const int controlPointIndex = fbxMesh->GetPolygonVertex(polyIndex, vertInFace);

                    FbxVector4 cp = fbxMesh->GetControlPointAt(controlPointIndex);
                    FbxVector4 pos4 = cp;
                    pos4[3] = 1.0;
                    FbxVector4 localPos = geometryTransform.MultT(pos4);
                    FbxVector4 localNormalRef;

                    {
                        FbxVector4 worldPos = meshNodeGlobal.MultT(localPos);
                        localPos = modelRootGlobalInv.MultT(worldPos);
                    }
                    section.Positions[vertexIndex] = ToVector3f(localPos);

                    FbxVector4 normal;
                    fbxMesh->GetPolygonVertexNormal(polyIndex, vertInFace, normal);
                    localNormalRef = normal;
                    {
                        FbxVector4 worldNormal = meshNodeGlobal.MultT(FbxVector4(normal[0], normal[1], normal[2], 0.0));
                        localNormalRef = modelRootGlobalInv.MultT(worldNormal);
                    }
                    section.Normals[vertexIndex] = ToVector3f(localNormalRef);

                    if (!recomputeTangents && hasTangents)
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

        if (sections.empty())
            return {nullptr, nullptr};

        // 找根骨骼节点（parentIndex == -1 且 FbxNode 父节点不在 nodeToIndex 里）
        int rootIndex = -1;
        FbxNode* rootBoneNode = nullptr;
        for (auto& [node, idx] : nodeToIndex)
        {
            if (allBones[idx].ParentIndex == -1)
            {
                rootIndex = idx;
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
            skeleton = Skeleton::StaticCreate(skeletonName, std::move(allBones), rootIndex);
            if (rootBoneNode)
            {
                SkeletonCacheEntry entry;
                entry.Skeleton    = skeleton;
                entry.NodeToIndex = std::move(nodeToIndex);
                Matrix4f modelRootWorld = ToMatrix4f(fbxNode->EvaluateGlobalTransform(FBXSDK_TIME_ZERO));
                entry.RebuiltBones = ReconstructPulsarSkeletalHierarchy(entry.NodeToIndex, skeleton->GetBones(), &modelRootWorld);

                static bool s_loggedImportCanonicalBoundary = false;
                if (!s_loggedImportCanonicalBoundary)
                {
                    Logger::Log(
                        "FBXImporter stage-boundary debug - meshRoot='" + string(fbxNode->GetName()) +
                        "', modelRootWorldT=(" + std::to_string(modelRootWorld[3][0]) + "," + std::to_string(modelRootWorld[3][1]) + "," + std::to_string(modelRootWorld[3][2]) + ")",
                        LogLevel::Warning);

                    for (const auto& [rebuiltNode, rebuiltInfo] : entry.RebuiltBones)
                    {
                        if (!rebuiltNode || rebuiltInfo.Index >= 4)
                            continue;

                        Logger::Log(
                            "FBXImporter stage-boundary bone debug - bone='" + string(rebuiltNode->GetName()) +
                            "', sourceLocalT=(" + std::to_string(rebuiltInfo.SourceLocalTranslation.x) + "," + std::to_string(rebuiltInfo.SourceLocalTranslation.y) + "," + std::to_string(rebuiltInfo.SourceLocalTranslation.z) + ")" +
                            ", sourceLocalS=(" + std::to_string(rebuiltInfo.SourceLocalScale.x) + "," + std::to_string(rebuiltInfo.SourceLocalScale.y) + "," + std::to_string(rebuiltInfo.SourceLocalScale.z) + ")" +
                            ", rebuiltLocalT=(" + std::to_string(rebuiltInfo.LocalTranslation.x) + "," + std::to_string(rebuiltInfo.LocalTranslation.y) + "," + std::to_string(rebuiltInfo.LocalTranslation.z) + ")" +
                            ", rebuiltLocalS=(" + std::to_string(rebuiltInfo.LocalScale.x) + "," + std::to_string(rebuiltInfo.LocalScale.y) + "," + std::to_string(rebuiltInfo.LocalScale.z) + ")" +
                            ", bindModelT=(" + std::to_string(rebuiltInfo.ModelMatrix[3][0]) + "," + std::to_string(rebuiltInfo.ModelMatrix[3][1]) + "," + std::to_string(rebuiltInfo.ModelMatrix[3][2]) + ")",
                            LogLevel::Warning);
                    }
                    s_loggedImportCanonicalBoundary = true;
                }
                string rootPath = rootBoneNode->GetName();
                entry.RootPath = std::move(rootPath);
                skeletonCache[rootBoneNode] = std::move(entry);
            }
        }

        auto mesh = SkinnedMesh::StaticCreate(name, skeleton, std::move(sections), std::move(materialNames));
        return {mesh, skeleton};
    }

    // -----------------------------------------------------------------------
    // ProcessMesh（StaticMesh，原有逻辑不变）
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

    // -----------------------------------------------------------------------
    // 辅助：提取节点 TRS 并应用到 transform
    // -----------------------------------------------------------------------
    static void ApplyNodeTransform(FbxNode* fbxNode, Node* node, const RebuiltBoneLookup* rebuiltBones = nullptr)
    {
        Vector3f translation;
        Vector3f scaling;
        Quat4f rotation;

        if (IsSkeletonNode(fbxNode))
        {
            const BoneRebuildInfo* rebuiltInfo = nullptr;
            if (rebuiltBones)
            {
                auto rebuildIt = rebuiltBones->find(fbxNode);
                if (rebuildIt != rebuiltBones->end())
                    rebuiltInfo = rebuildIt->second;
            }

            if (rebuiltInfo)
            {
                translation = rebuiltInfo->SourceLocalTranslation;
                scaling = rebuiltInfo->SourceLocalScale;
                rotation = rebuiltInfo->LocalRotation;

                static int s_loggedSkeletonCompare = 0;
                if (s_loggedSkeletonCompare < 8)
                {
                    const FbxAMatrix localEval = EvaluateReconstructedLocalTransform(fbxNode, FBXSDK_TIME_ZERO);
                    const FbxAMatrix globalEval = fbxNode->EvaluateGlobalTransform(FBXSDK_TIME_ZERO);
                    const auto localEvalQ = ToQuat(localEval.GetQ());
                    const auto globalEvalQ = ToQuat(globalEval.GetQ());
                    const auto localEvalEuler = localEvalQ.GetEuler(jmath::EulerOrder::YXZ);
                    const auto globalEvalEuler = globalEvalQ.GetEuler(jmath::EulerOrder::YXZ);
                    const auto rebuiltEuler = rotation.GetEuler(jmath::EulerOrder::YXZ);

                    Vector3f childDir{0.f, 0.f, 0.f};
                    for (int childIdx = 0; childIdx < fbxNode->GetChildCount(); ++childIdx)
                    {
                        FbxNode* child = fbxNode->GetChild(childIdx);
                        if (!IsSkeletonNode(child))
                            continue;
                        Vector3f p = ToVector3f(globalEval.GetT());
                        Vector3f c = ToVector3f(child->EvaluateGlobalTransform(FBXSDK_TIME_ZERO).GetT());
                        childDir = Normalize(c - p);
                        break;
                    }

                    Logger::Log(
                        string("FBXImporter skeleton compare - name='") + fbxNode->GetName() +
                        "', evalLocalQ=(" + std::to_string(localEvalQ.x) + "," + std::to_string(localEvalQ.y) + "," + std::to_string(localEvalQ.z) + "," + std::to_string(localEvalQ.w) + ")" +
                        ", evalLocalEulerYXZ=(" + std::to_string(localEvalEuler.x) + "," + std::to_string(localEvalEuler.y) + "," + std::to_string(localEvalEuler.z) + ")" +
                        ", evalGlobalQ=(" + std::to_string(globalEvalQ.x) + "," + std::to_string(globalEvalQ.y) + "," + std::to_string(globalEvalQ.z) + "," + std::to_string(globalEvalQ.w) + ")" +
                        ", evalGlobalEulerYXZ=(" + std::to_string(globalEvalEuler.x) + "," + std::to_string(globalEvalEuler.y) + "," + std::to_string(globalEvalEuler.z) + ")" +
                        ", rebuiltQ=(" + std::to_string(rotation.x) + "," + std::to_string(rotation.y) + "," + std::to_string(rotation.z) + "," + std::to_string(rotation.w) + ")" +
                        ", rebuiltEulerYXZ=(" + std::to_string(rebuiltEuler.x) + "," + std::to_string(rebuiltEuler.y) + "," + std::to_string(rebuiltEuler.z) + ")" +
                        ", childDir=(" + std::to_string(childDir.x) + "," + std::to_string(childDir.y) + "," + std::to_string(childDir.z) + ")",
                        LogLevel::Warning);
                    ++s_loggedSkeletonCompare;
                }
            }
            else
            {
                auto local = EvaluateReconstructedLocalTransform(fbxNode, FBXSDK_TIME_ZERO);
                translation = ToVector3f(local.GetT());
                scaling     = ToVector3f(fbxNode->LclScaling.Get());

                FbxNode* parent = fbxNode->GetParent();
                const Vector3f rootUpRef{0.f, -1.f, 0.f};
                Matrix4f worldBasis = (!parent || !IsSkeletonNode(parent))
                    ? BuildBasisFromForward(-ComputePulsarBoneForward(fbxNode), &rootUpRef)
                    : BuildBasisFromForward(-ComputePulsarBoneForward(fbxNode));
                worldBasis[3][0] = 0.f;
                worldBasis[3][1] = 0.f;
                worldBasis[3][2] = 0.f;
                worldBasis[3][3] = 1.f;
                if (parent && IsSkeletonNode(parent))
                {
                    Matrix4f parentWorldBasis = parent->GetParent() && IsSkeletonNode(parent->GetParent())
                        ? BuildBasisFromForward(-ComputePulsarBoneForward(parent))
                        : BuildBasisFromForward(-ComputePulsarBoneForward(parent), &rootUpRef);
                    parentWorldBasis[3][0] = 0.f;
                    parentWorldBasis[3][1] = 0.f;
                    parentWorldBasis[3][2] = 0.f;
                    parentWorldBasis[3][3] = 1.f;
                    Matrix4f localBasis = jmath::Inverse(parentWorldBasis) * worldBasis;
                    localBasis[3][0] = 0.f;
                    localBasis[3][1] = 0.f;
                    localBasis[3][2] = 0.f;
                    localBasis[3][3] = 1.f;
                    rotation = MatrixToQuat(localBasis);
                    {
                        float qlen = std::sqrt(
                            rotation.x * rotation.x +
                            rotation.y * rotation.y +
                            rotation.z * rotation.z +
                            rotation.w * rotation.w);
                        if (qlen > 1e-8f)
                        {
                            rotation.x /= qlen;
                            rotation.y /= qlen;
                            rotation.z /= qlen;
                            rotation.w /= qlen;
                        }
                    }
                }
                else
                {
                    rotation = MatrixToQuat(worldBasis);
                    {
                        float qlen = std::sqrt(
                            rotation.x * rotation.x +
                            rotation.y * rotation.y +
                            rotation.z * rotation.z +
                            rotation.w * rotation.w);
                        if (qlen > 1e-8f)
                        {
                            rotation.x /= qlen;
                            rotation.y /= qlen;
                            rotation.z /= qlen;
                            rotation.w /= qlen;
                        }
                    }
                }
            }
        }
        else
        {
            translation = ToVector3f(fbxNode->LclTranslation.Get());
            scaling     = ToVector3f(fbxNode->LclScaling.Get());
            rotation    = Quat4f::FromEuler(ToVector3f(fbxNode->LclRotation.Get()));
        }

        auto transform = node->GetTransform();
        transform->SetPosition(translation);
        transform->SetRotation(rotation);
        transform->SetScale(scaling);

        static int s_loggedAppliedNodeTransforms = 0;
        if (s_loggedAppliedNodeTransforms < 8)
        {
            auto euler = rotation.GetEuler(jmath::EulerOrder::YXZ);
            Logger::Log(
                string("FBXImporter applied node transform - name='") + fbxNode->GetName() +
                "', kind='" + (IsSkeletonNode(fbxNode) ? string("Skeleton") : string("NodeOrMesh")) +
                "', pos=(" + std::to_string(translation.x) + "," + std::to_string(translation.y) + "," + std::to_string(translation.z) + ")" +
                ", rotQuat=(" + std::to_string(rotation.x) + "," + std::to_string(rotation.y) + "," + std::to_string(rotation.z) + "," + std::to_string(rotation.w) + ")" +
                ", rotEulerYXZ=(" + std::to_string(euler.x) + "," + std::to_string(euler.y) + "," + std::to_string(euler.z) + ")" +
                ", scale=(" + std::to_string(scaling.x) + "," + std::to_string(scaling.y) + "," + std::to_string(scaling.z) + ")",
                LogLevel::Warning);
            ++s_loggedAppliedNodeTransforms;
        }
    }

    static FbxNode* FindRootBoneNode(FbxMesh* fbxMesh)
    {
        if (!fbxMesh)
            return nullptr;

        for (int deformerIndex = 0; deformerIndex < fbxMesh->GetDeformerCount(FbxDeformer::eSkin); ++deformerIndex)
        {
            auto skin = static_cast<FbxSkin*>(fbxMesh->GetDeformer(deformerIndex, FbxDeformer::eSkin));
            if (!skin)
                continue;

            for (int clusterIndex = 0; clusterIndex < skin->GetClusterCount(); ++clusterIndex)
            {
                auto cluster = skin->GetCluster(clusterIndex);
                if (!cluster)
                    continue;

                auto link = cluster->GetLink();
                if (!link)
                    continue;

                auto root = link;
                while (root->GetParent() && root->GetParent()->GetSkeleton())
                    root = root->GetParent();
                return root;
            }
        }

        return nullptr;
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

    static void ProcessNode(
        FbxNode* fbxNode,
        ObjectPtr<Node> parentNode,
        RCPtr<Prefab> pscene,
        FBXImporterSettings* settings,
        bool inverseCoordsystem,
        const string& meshFolder,
        array_list<RCPtr<AssetObject>>& importedAssets,
        SkeletonCache& skeletonCache,
        PendingRootBoneBindings& pendingRootBoneBindings,
        const RebuiltBoneLookup* rebuiltBones
        )
    {
        auto newNodeName = fbxNode->GetName();
        const auto newNode = pscene->NewNode(newNodeName, parentNode);
        ApplyNodeTransform(fbxNode, newNode.GetPtr(), rebuiltBones);

        bool hasSkeletonAttr = false;
        bool hasMeshAttr = false;
        string attrTypes;
        for (int attrIndex = 0; attrIndex < fbxNode->GetNodeAttributeCount(); attrIndex++)
        {
            auto attr = fbxNode->GetNodeAttributeByIndex(attrIndex);
            if (!attr) continue;
            if (!attrTypes.empty()) attrTypes += ",";
            attrTypes += std::to_string((int)attr->GetAttributeType());
            hasSkeletonAttr = hasSkeletonAttr || attr->GetAttributeType() == FbxNodeAttribute::eSkeleton;
            hasMeshAttr = hasMeshAttr || attr->GetAttributeType() == FbxNodeAttribute::eMesh;
        }
        Logger::Log(
            string("FBXImporter node: name='") + newNodeName
            + "', parent='" + (parentNode ? parentNode->GetName() : string("<null>"))
            + "', attrs='" + attrTypes
            + "', childCount='" + std::to_string(fbxNode->GetChildCount()) + "'",
            LogLevel::Warning);

        LogNodeRotationDebug(fbxNode, "post-convert");

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
            auto [skinnedMesh, skeleton] = ProcessSkinnedMesh(fbxNode, inverseCoordsystem, skeletonCache, settings->RecomputeTangents, settings->UseMikktspace);
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
                ApplyMaterialPolicyImpl(skinnedMesh->GetMaterialCount(), skinnedMesh->GetMaterialNames(), settings, meshFolder, importedAssets,
                    [renderer](int idx, RCPtr<Material> mat) { renderer->SetMaterial(idx, mat); });

                if (skeleton)
                {
                    pendingRootBoneBindings.push_back(PendingRootBoneBinding
                    {
                        .MeshNode = newNode,
                        .Renderer = renderer,
                        .Skeleton = skeleton
                    });
                }

            }
        }
        else
        {
            // StaticMesh 路径（原有逻辑）
            if (auto staticMesh = ProcessMesh(fbxNode, inverseCoordsystem, settings->RecomputeTangents, settings->UseMikktspace))
            {
                const auto meshPath = meshFolder + "/" + staticMesh->GetName();
                AssetDatabase::CreateAsset(staticMesh, meshPath);
                auto renderer = newNode->AddComponent<StaticMeshRendererComponent>();
                renderer->SetStaticMesh(staticMesh);
                ApplyMaterialPolicyImpl(staticMesh->GetMaterialCount(), staticMesh->GetMaterialNames(), settings, meshFolder, importedAssets,
                    [renderer](int idx, RCPtr<Material> mat) { renderer->SetMaterial(idx, mat); });
                importedAssets.push_back(staticMesh);
            }
        }

        const auto childCount = fbxNode->GetChildCount();
        for (int childIndex = 0; childIndex < childCount; childIndex++)
        {
            const auto childFbxNode = fbxNode->GetChild(childIndex);
            ProcessNode(childFbxNode, newNode, pscene, settings, inverseCoordsystem, meshFolder, importedAssets, skeletonCache, pendingRootBoneBindings, rebuiltBones);
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
                LogNodeRotationDebugRecursive(fbxScene->GetRootNode(), "pre-convert");
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
                LogNodeRotationDebugRecursive(fbxScene->GetRootNode(), "post-convert-scene");
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
            const auto filename = PathUtil::GetFilenameWithoutExt(importFile.filename().string());
            // Prefab 内根节点名 = 纯文件名（如 TestSkinned），两者不同名
            auto prefab = Prefab::StaticCreate(filename + "Prefab");
            const auto targetMeshFolder = settings->ImportingTargetFolder;
            const auto rootCount = fbxRootNode->GetChildCount();
            SkeletonCache skeletonCache; // 整个 FBX 文件共享同一套骨骼缓存
            PendingRootBoneBindings pendingRootBoneBindings;

            for (int i = 0; i < rootCount; ++i)
            {
                auto sceneRootNode = fbxRootNode->GetChild(i);
                if (!sceneRootNode)
                    continue;

                if (auto meshAttr = sceneRootNode->GetMesh())
                {
                    if (auto rootBone = FindRootBoneNode(meshAttr))
                    {
                        ProcessSkinnedMesh(sceneRootNode, inverseCoordSystem, skeletonCache, fbxsetting->RecomputeTangents, fbxsetting->UseMikktspace);
                    }
                }
            }

            RebuiltBoneLookup rebuiltBoneLookup;
            for (const auto& [rootBone, entry] : skeletonCache)
            {
                for (const auto& [boneNode, rebuildInfo] : entry.RebuiltBones)
                {
                    rebuiltBoneLookup[boneNode] = &rebuildInfo;
                }
            }

            // Prefab 只有一个根节点，名字为纯文件名
            auto prefabRoot = prefab->NewNode(filename, nullptr);
            for (int i = 0; i < rootCount; ++i)
            {
                auto sceneRootNode = fbxRootNode->GetChild(i);
                ProcessNode(sceneRootNode, prefabRoot, prefab, fbxsetting, inverseCoordSystem, targetMeshFolder, importedAssets, skeletonCache, pendingRootBoneBindings, &rebuiltBoneLookup);
            }

            for (auto& binding : pendingRootBoneBindings)
            {
                if (!binding.Renderer || !binding.MeshNode || !binding.Skeleton)
                    continue;

                const auto& skeletonBones = binding.Skeleton->GetBones();
                if (skeletonBones.empty())
                    continue;

                auto rootBoneTransform = !skeletonBones[0].Path.empty()
                    ? prefabRoot->GetTransform()->FindByPath(skeletonBones[0].Path)
                    : prefabRoot->GetTransform()->FindByName(skeletonBones[0].Name);

                if (!rootBoneTransform)
                {
                    rootBoneTransform = binding.MeshNode->GetTransform()->FindByName(skeletonBones[0].Name);
                }

                string childNames;
                const auto& children = binding.MeshNode->GetTransform()->GetChildren();
                for (int i = 0; i < (int)children->size(); ++i)
                {
                    if (i > 0) childNames += ",";
                    childNames += children->at(i)->GetNode()->GetName();
                }

                Logger::Log(
                    string("FBXImporter root bone resolve: mesh='") + binding.MeshNode->GetName()
                    + "', bone0.name='" + skeletonBones[0].Name
                    + "', bone0.path='" + skeletonBones[0].Path
                    + "', children='" + childNames
                    + "', found='" + (rootBoneTransform ? rootBoneTransform->GetNode()->GetName() : string("<null>"))
                    + "'",
                    rootBoneTransform ? LogLevel::Warning : LogLevel::Error);

                binding.Renderer->SetRoot(prefabRoot->GetTransform());
            }

            // 如果有骨骼，在 prefabRoot 上加 AnimatorComponent
            if (!skeletonCache.empty())
            {
                prefabRoot->AddComponent<AnimatorComponent>();
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

                if (fbxsetting->ImportAnimationClips)
                {
                    auto clips = ExtractAnimationClips(fbxScene, entry, baseName);
                    for (auto& clip : clips)
                    {
                        const auto clipPath = targetMeshFolder + "/" + clip->GetName();
                        AssetDatabase::CreateAsset(clip, clipPath);
                        importedAssets.push_back(pulsar::RCPtr<pulsar::AssetObject>(clip));
                    }
                }
            }

            AssetDatabase::CreateAsset(prefab, settings->ImportingTargetFolder + "/" + filename + "Prefab");
            importedAssets.push_back(prefab);

            DestroySdkObjects(fbxManager, 0);
        }
        return importedAssets;
    }

} // namespace pulsared
