#include "Importers/FBXImporterInternal.h"
#include <Pulsar/TransformUtil.h>
#include <Pulsar/Logger.h>

namespace pulsared
{
    bool HasSkinDeformer(FbxMesh* fbxMesh)
    {
        return fbxMesh->GetDeformerCount(FbxDeformer::eSkin) > 0;
    }

    // -----------------------------------------------------------------------
    // ProcessSkinnedMesh（SkinnedMesh）
    // -----------------------------------------------------------------------
    RCPtr<SkinnedMesh> ProcessSkinnedMesh(FbxNode* fbxNode, RCPtr<Skeleton> skeleton, bool inverseCoordsystem, bool recomputeTangents, bool useMikktspace)
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

    RCPtr<Skeleton> ProcessSkeleton(FbxScene* fbxScene, const string& name, bool inverseCoordSystem)
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
}
