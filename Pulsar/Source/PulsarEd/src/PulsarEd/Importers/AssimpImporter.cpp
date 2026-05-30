#include "Importers/AssimpImporter.h"
#include "Importers/FBXImporterInternal.h" // for tangent generation helpers

#include "Pulsar/Prefab.h"
#include "Pulsar/Scene.h"
#include "Pulsar/Logger.h"
#include "Pulsar/TransformUtil.h"
#include "Pulsar/AssetManager.h"
#include "Pulsar/Assets/NodeCollection.h"
#include "Pulsar/Assets/Material.h"
#include "Pulsar/Assets/AnimationClip.h"
#include "Pulsar/Components/StaticMeshRendererComponent.h"
#include "Pulsar/Components/SkinnedMeshRendererComponent.h"
#include "PulsarEd/AssetDatabase.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include <assimp/matrix4x4.h>
#include <assimp/material.h>
#include <functional>
#include <algorithm>

namespace pulsared
{
    // -----------------------------------------------------------------------
    // Helpers: type conversions
    // -----------------------------------------------------------------------
    static inline Vector3f ToVector3f(const aiVector3D& v)
    {
        return { v.x, v.y, v.z };
    }

    static inline Vector2f ToVector2f(const aiVector3D& v)
    {
        return { v.x, v.y };
    }

    static inline Color4b ToColor(const aiColor4D& c)
    {
        auto f2u8 = [](float v) -> uint8_t {
            int iv = static_cast<int>(v * 255.0f + 0.5f);
            return static_cast<uint8_t>(std::clamp(iv, 0, 255));
        };
        return Color4b{ f2u8(c.r), f2u8(c.g), f2u8(c.b), f2u8(c.a) };
    }

    static inline Quat4f ToQuat(const aiQuaternion& q)
    {
        return { q.x, q.y, q.z, q.w };
    }

    static inline Matrix4f ToMatrix4f(const aiMatrix4x4& m)
    {
        Matrix4f result;
        // aiMatrix4x4 is row-major, Pulsar Matrix4f is col-major (glm-style)
        for (int i = 0; i < 4; ++i)
            for (int j = 0; j < 4; ++j)
                result[i][j] = m[j][i];
        return result;
    }

    static inline void DecomposeTRS(const aiMatrix4x4& m, Vector3f& outT, Quat4f& outR, Vector3f& outS)
    {
        aiVector3D t, s;
        aiQuaternion r;
        m.Decompose(s, r, t);
        outT = ToVector3f(t);
        outR = ToQuat(r);
        outS = ToVector3f(s);
    }

    static string GetMaterialName(const aiScene* scene, unsigned int materialIndex)
    {
        if (materialIndex < scene->mNumMaterials)
        {
            aiMaterial* mat = scene->mMaterials[materialIndex];
            aiString name;
            if (AI_SUCCESS == aiGetMaterialString(mat, AI_MATKEY_NAME, &name))
                return string(name.C_Str());
        }
        return string();
    }

    // -----------------------------------------------------------------------
    // Helpers: material policy
    // -----------------------------------------------------------------------
    static void ApplyMaterialPolicy(
        size_t materialCount,
        const array_list<string>& materialNames,
        AssimpImporterSettings* settings,
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
                auto shader = AssetManager::Get()->LoadAsset<Shader>("Engine/Shaders/Lambert");
                if (shader)
                {
                    foundMaterial = Material::StaticCreate(shader, matName);
                    AssetDatabase::CreateAsset(foundMaterial, AssetDatabase::GetUniquePath(meshFolder + "/" + matName));
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
    // Skeleton extraction
    // -----------------------------------------------------------------------
    struct AssimpBoneInfo
    {
        string Name;
        const aiNode* Node = nullptr;
        int32_t ParentIndex = -1;
    };

    static bool IsBoneNode(const aiNode* node, const hash_set<string>& boneNames)
    {
        return boneNames.find(string(node->mName.C_Str())) != boneNames.end();
    }

    static void CollectSkeletonNodes(
        const aiNode* node,
        const hash_set<string>& boneNames,
        array_list<AssimpBoneInfo>& outInfos,
        int32_t parentIndex)
    {
        bool isBone = IsBoneNode(node, boneNames);
        int32_t myIndex = parentIndex;
        if (isBone)
        {
            myIndex = static_cast<int32_t>(outInfos.size());
            outInfos.push_back({ string(node->mName.C_Str()), node, parentIndex });
        }
        for (uint32_t i = 0; i < node->mNumChildren; ++i)
        {
            CollectSkeletonNodes(node->mChildren[i], boneNames, outInfos, myIndex);
        }
    }

    static RCPtr<Skeleton> BuildSkeleton(const aiScene* scene, const string& name)
    {
        // Collect all bone names from all meshes
        hash_set<string> boneNames;
        for (uint32_t m = 0; m < scene->mNumMeshes; ++m)
        {
            aiMesh* mesh = scene->mMeshes[m];
            for (uint32_t b = 0; b < mesh->mNumBones; ++b)
            {
                boneNames.insert(string(mesh->mBones[b]->mName.C_Str()));
            }
        }
        if (boneNames.empty())
            return nullptr;

        array_list<AssimpBoneInfo> skeletonNodes;
        CollectSkeletonNodes(scene->mRootNode, boneNames, skeletonNodes, -1);
        if (skeletonNodes.empty())
            return nullptr;

        // Find root bone index
        int32_t rootBoneIndex = -1;
        for (int32_t i = 0; i < (int32_t)skeletonNodes.size(); ++i)
        {
            if (skeletonNodes[i].ParentIndex == -1)
            {
                rootBoneIndex = i;
                break;
            }
        }
        if (rootBoneIndex < 0)
            rootBoneIndex = 0;

        // Compute global bind pose for each bone
        hash_map<string, Matrix4f> nodeGlobalMatrices;
        std::function<void(const aiNode*, const Matrix4f&)> computeGlobal = [&](const aiNode* node, const Matrix4f& parentGlobal)
        {
            Matrix4f local = ToMatrix4f(node->mTransformation);
            Matrix4f global = parentGlobal * local;
            nodeGlobalMatrices[string(node->mName.C_Str())] = global;
            for (uint32_t i = 0; i < node->mNumChildren; ++i)
                computeGlobal(node->mChildren[i], global);
        };
        computeGlobal(scene->mRootNode, Matrix4f(1.f));

        // Get root bone global matrix for canonical model space
        Matrix4f rootGlobalBind = nodeGlobalMatrices[skeletonNodes[rootBoneIndex].Name];
        Matrix4f rootGlobalBindInv = jmath::Inverse(rootGlobalBind);

        array_list<BoneInfo> bones;
        bones.reserve(skeletonNodes.size());

        for (int32_t i = 0; i < (int32_t)skeletonNodes.size(); ++i)
        {
            const auto& info = skeletonNodes[i];
            BoneInfo bone;
            bone.Name = info.Name;

            // Build relative Path from skeleton root's children
            {
                array_list<string> pathParts;
                int32_t idx = i;
                while (idx >= 0)
                {
                    pathParts.push_back(skeletonNodes[idx].Name);
                    idx = skeletonNodes[idx].ParentIndex;
                }
                std::reverse(pathParts.begin(), pathParts.end());
                // Drop root bone name since search starts from m_root itself
                if (!pathParts.empty())
                    pathParts.erase(pathParts.begin());
                bone.Path = pathParts.empty() ? "" : StringUtil::Join(pathParts, "/");
            }

            bone.ParentIndex = info.ParentIndex;

            // Global bind pose in scene space
            auto it = nodeGlobalMatrices.find(info.Name);
            Matrix4f globalBind = (it != nodeGlobalMatrices.end()) ? it->second : Matrix4f(1.f);

            // BindModelMatrix relative to skeleton root
            Matrix4f bindModelMatrix = rootGlobalBindInv * globalBind;

            // Extract local TRS from bindModelMatrix
            if (bone.ParentIndex >= 0)
            {
                Matrix4f parentBindModel = bones[bone.ParentIndex].BindModelMatrix;
                Matrix4f localMat = jmath::Inverse(parentBindModel) * bindModelMatrix;
                // Convert Pulsar col-major back to aiMatrix4x4 row-major for Decompose
                aiMatrix4x4 aim(
                    localMat[0][0], localMat[1][0], localMat[2][0], localMat[3][0],
                    localMat[0][1], localMat[1][1], localMat[2][1], localMat[3][1],
                    localMat[0][2], localMat[1][2], localMat[2][2], localMat[3][2],
                    localMat[0][3], localMat[1][3], localMat[2][3], localMat[3][3]
                );
                DecomposeTRS(aim, bone.LocalTranslation, bone.LocalRotation, bone.LocalScale);
            }
            else
            {
                aiMatrix4x4 aim(
                    bindModelMatrix[0][0], bindModelMatrix[1][0], bindModelMatrix[2][0], bindModelMatrix[3][0],
                    bindModelMatrix[0][1], bindModelMatrix[1][1], bindModelMatrix[2][1], bindModelMatrix[3][1],
                    bindModelMatrix[0][2], bindModelMatrix[1][2], bindModelMatrix[2][2], bindModelMatrix[3][2],
                    bindModelMatrix[0][3], bindModelMatrix[1][3], bindModelMatrix[2][3], bindModelMatrix[3][3]
                );
                DecomposeTRS(aim, bone.LocalTranslation, bone.LocalRotation, bone.LocalScale);
            }

            bones.push_back(bone);
        }

        // Second pass: compute BindModelMatrix from local TRS to ensure consistency
        for (int32_t i = 0; i < (int32_t)bones.size(); ++i)
        {
            auto& bone = bones[i];
            Matrix4f localMat;
            transutil::NewTRS(localMat, bone.LocalTranslation, bone.LocalRotation, bone.LocalScale);
            if (bone.ParentIndex >= 0)
                bone.BindModelMatrix = bones[bone.ParentIndex].BindModelMatrix * localMat;
            else
                bone.BindModelMatrix = localMat;
        }

        // Third pass: InverseBindMatrix must be based on world-space bind pose (globalBind),
        // not the relative-to-root matrix. The second-pass BindModelMatrix is kept for
        // compatibility but InverseBindMatrix is overridden from the world matrix.
        for (int32_t i = 0; i < (int32_t)bones.size(); ++i)
        {
            auto& bone = bones[i];
            auto it = nodeGlobalMatrices.find(bone.Name);
            if (it != nodeGlobalMatrices.end())
            {
                bone.BindModelMatrix = it->second;
                bone.InverseBindMatrix = jmath::Inverse(it->second);
            }
            else
            {
                bone.InverseBindMatrix = jmath::Inverse(bone.BindModelMatrix);
            }
        }

        return Skeleton::StaticCreate(name, std::move(bones), rootBoneIndex);
    }

    // -----------------------------------------------------------------------
    // Mesh processing
    // -----------------------------------------------------------------------
    static RCPtr<StaticMesh> ProcessStaticMesh(
        const aiScene* scene,
        const aiMesh* mesh,
        const string& name,
        bool recomputeTangents,
        bool useMikktspace)
    {
        StaticMeshSection section;
        const uint32_t numUV = std::min(mesh->GetNumUVChannels(), (uint32_t)STATICMESH_MAX_TEXTURE_COORDS);
        section.NumTexCoords = static_cast<uint8_t>(numUV);

        const uint32_t vertexCount = mesh->mNumVertices;
        section.Positions.resize(vertexCount);
        section.Normals.resize(vertexCount);
        section.TexCoords.resize(numUV);
        for (uint32_t uvIdx = 0; uvIdx < numUV; ++uvIdx)
            section.TexCoords[uvIdx].resize(vertexCount);

        const bool hasColors = mesh->HasVertexColors(0);
        const bool hasTangents = mesh->HasTangentsAndBitangents();
        if (hasColors) section.Colors.resize(vertexCount);
        if (recomputeTangents || hasTangents) section.Tangents.resize(vertexCount);

        for (uint32_t i = 0; i < vertexCount; ++i)
        {
            section.Positions[i] = ToVector3f(mesh->mVertices[i]);
            section.Normals[i] = ToVector3f(mesh->mNormals[i]);
            for (uint32_t uvIdx = 0; uvIdx < numUV; ++uvIdx)
                section.TexCoords[uvIdx][i] = ToVector2f(mesh->mTextureCoords[uvIdx][i]);
            if (hasColors)
                section.Colors[i] = ToColor(mesh->mColors[0][i]);
            if (!recomputeTangents && hasTangents)
            {
                const Vector3f T = ToVector3f(mesh->mTangents[i]);
                const Vector3f B = ToVector3f(mesh->mBitangents[i]);
                const Vector3f N = section.Normals[i];
                float w = Dot(Cross(N, T), B) > 0.0f ? 1.0f : -1.0f;
                section.Tangents[i] = Vector4f{ T.x, T.y, T.z, w };
            }
        }

        // Faces
        // Flip winding order: aiProcess_ConvertToLeftHanded produces CW faces,
        // but Pulsar/Vulkan expects CCW front faces.
        const uint32_t faceCount = mesh->mNumFaces;
        section.Indices.reserve(faceCount * 3);
        for (uint32_t i = 0; i < faceCount; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            if (face.mNumIndices != 3)
                continue;
            section.Indices.push_back(static_cast<MeshIndicesType>(face.mIndices[0]));
            section.Indices.push_back(static_cast<MeshIndicesType>(face.mIndices[2]));
            section.Indices.push_back(static_cast<MeshIndicesType>(face.mIndices[1]));
        }

        if (recomputeTangents && numUV > 0 && !section.Positions.empty())
        {
            if (useMikktspace)
                GenerateTangentsForSection(section.Positions, section.Normals, section.TexCoords[0], section.Tangents);
            else
                GenerateSimpleTangentsForSection(section.Positions, section.Normals, section.TexCoords[0], section.Tangents);
        }

        array_list<StaticMeshSection> sections;
        sections.push_back(std::move(section));

        array_list<string> materialNames;
        string matName = GetMaterialName(scene, mesh->mMaterialIndex);
        if (!matName.empty())
            materialNames.push_back(matName);

        return StaticMesh::StaticCreate(name, std::move(sections), std::move(materialNames));
    }

    static RCPtr<SkinnedMesh> ProcessSkinnedMesh(
        const aiScene* scene,
        const aiMesh* mesh,
        const string& name,
        RCPtr<Skeleton> skeleton,
        bool recomputeTangents,
        bool useMikktspace)
    {
        if (!skeleton)
            return nullptr;

        SkinnedMeshSection section;
        const uint32_t numUV = std::min(mesh->GetNumUVChannels(), (uint32_t)STATICMESH_MAX_TEXTURE_COORDS);
        section.NumTexCoords = static_cast<uint8_t>(numUV);

        const uint32_t vertexCount = mesh->mNumVertices;
        section.Positions.resize(vertexCount);
        section.Normals.resize(vertexCount);
        section.TexCoords.resize(numUV);
        for (uint32_t uvIdx = 0; uvIdx < numUV; ++uvIdx)
            section.TexCoords[uvIdx].resize(vertexCount);

        const bool hasColors = mesh->HasVertexColors(0);
        const bool hasTangents = mesh->HasTangentsAndBitangents();
        if (hasColors) section.Colors.resize(vertexCount);
        if (recomputeTangents || hasTangents) section.Tangents.resize(vertexCount);

        // Skinning data
        section.BoneIndices.resize(vertexCount);
        section.BoneWeights.resize(vertexCount);
        array_list<array_list<std::pair<int, float>>> vertInfluences;
        vertInfluences.resize(vertexCount);

        for (uint32_t b = 0; b < mesh->mNumBones; ++b)
        {
            aiBone* bone = mesh->mBones[b];
            string boneName(bone->mName.C_Str());
            int boneIndex = skeleton->FindBoneIndex(boneName);
            if (boneIndex < 0)
            {
                Logger::Log(StringUtil::Concat("Warning: AssimpImporter bone '", boneName, "' not found in Skeleton"));
                continue;
            }
            for (uint32_t w = 0; w < bone->mNumWeights; ++w)
            {
                uint32_t vid = bone->mWeights[w].mVertexId;
                float weight = bone->mWeights[w].mWeight;
                if (vid < vertexCount && weight > 0.0f)
                    vertInfluences[vid].push_back({ boneIndex, weight });
            }
        }

        // Sort, clamp to 4, normalize
        for (auto& influences : vertInfluences)
        {
            std::sort(influences.begin(), influences.end(), [](const auto& a, const auto& b) {
                return a.second > b.second;
            });
            if (influences.size() > SKINNEDMESH_MAX_BONE_INFLUENCES)
                influences.resize(SKINNEDMESH_MAX_BONE_INFLUENCES);
            float sum = 0.0f;
            for (const auto& [idx, w] : influences) sum += w;
            if (sum > 0.0f)
                for (auto& [idx, w] : influences) w /= sum;
        }

        for (uint32_t i = 0; i < vertexCount; ++i)
        {
            section.Positions[i] = ToVector3f(mesh->mVertices[i]);
            section.Normals[i] = ToVector3f(mesh->mNormals[i]);
            for (uint32_t uvIdx = 0; uvIdx < numUV; ++uvIdx)
                section.TexCoords[uvIdx][i] = ToVector2f(mesh->mTextureCoords[uvIdx][i]);
            if (hasColors)
                section.Colors[i] = ToColor(mesh->mColors[0][i]);
            if (!recomputeTangents && hasTangents)
            {
                const Vector3f T = ToVector3f(mesh->mTangents[i]);
                const Vector3f B = ToVector3f(mesh->mBitangents[i]);
                const Vector3f N = section.Normals[i];
                float w = Dot(Cross(N, T), B) > 0.0f ? 1.0f : -1.0f;
                section.Tangents[i] = Vector4f{ T.x, T.y, T.z, w };
            }

            std::array<uint32_t, SKINNEDMESH_MAX_BONE_INFLUENCES> boneIndices = {};
            std::array<float, SKINNEDMESH_MAX_BONE_INFLUENCES> boneWeights = {};
            for (size_t k = 0; k < vertInfluences[i].size() && k < SKINNEDMESH_MAX_BONE_INFLUENCES; ++k)
            {
                boneIndices[k] = static_cast<uint32_t>(vertInfluences[i][k].first);
                boneWeights[k] = vertInfluences[i][k].second;
            }
            section.BoneIndices[i] = boneIndices;
            section.BoneWeights[i] = boneWeights;
        }

        // Faces
        // Flip winding order: aiProcess_ConvertToLeftHanded produces CW faces,
        // but Pulsar/Vulkan expects CCW front faces.
        const uint32_t faceCount = mesh->mNumFaces;
        section.Indices.reserve(faceCount * 3);
        for (uint32_t i = 0; i < faceCount; ++i)
        {
            const aiFace& face = mesh->mFaces[i];
            if (face.mNumIndices != 3)
                continue;
            section.Indices.push_back(static_cast<MeshIndicesType>(face.mIndices[0]));
            section.Indices.push_back(static_cast<MeshIndicesType>(face.mIndices[2]));
            section.Indices.push_back(static_cast<MeshIndicesType>(face.mIndices[1]));
        }

        if (recomputeTangents && numUV > 0 && !section.Positions.empty())
        {
            if (useMikktspace)
                GenerateTangentsForSection(section.Positions, section.Normals, section.TexCoords[0], section.Tangents);
            else
                GenerateSimpleTangentsForSection(section.Positions, section.Normals, section.TexCoords[0], section.Tangents);
        }

        array_list<SkinnedMeshSection> sections;
        sections.push_back(std::move(section));

        array_list<string> materialNames;
        string matName = GetMaterialName(scene, mesh->mMaterialIndex);
        if (!matName.empty())
            materialNames.push_back(matName);

        return SkinnedMesh::StaticCreate(name, skeleton, std::move(sections), std::move(materialNames));
    }

    // -----------------------------------------------------------------------
    // Animation extraction
    // -----------------------------------------------------------------------
    static array_list<RCPtr<AnimationClip>> BuildAnimationClips(
        const aiScene* scene,
        RCPtr<Skeleton> skeleton,
        const string& filename)
    {
        array_list<RCPtr<AnimationClip>> clips;
        if (!scene->HasAnimations() || !skeleton)
            return clips;

        for (uint32_t animIdx = 0; animIdx < scene->mNumAnimations; ++animIdx)
        {
            aiAnimation* anim = scene->mAnimations[animIdx];
            string animName = anim->mName.length > 0 ? string(anim->mName.C_Str()) : (filename + "Anim" + std::to_string(animIdx));

            float duration = anim->mTicksPerSecond > 0.0
                ? static_cast<float>(anim->mDuration / anim->mTicksPerSecond)
                : static_cast<float>(anim->mDuration);
            float fps = anim->mTicksPerSecond > 0.0
                ? static_cast<float>(anim->mTicksPerSecond)
                : 30.0f;

            array_list<SPtr<AnimationTrack>> tracks;

            for (uint32_t chIdx = 0; chIdx < anim->mNumChannels; ++chIdx)
            {
                aiNodeAnim* channel = anim->mChannels[chIdx];
                string nodeName(channel->mNodeName.C_Str());

                // Check if this node is a bone in our skeleton
                int boneIndex = skeleton->FindBoneIndex(nodeName);
                if (boneIndex < 0)
                    continue; // Skip non-bone animation channels for now

                auto boneTrack = mksptr(new BoneAnimationTrack());
                boneTrack->TrackType = AnimationTrackType::Bone;
                boneTrack->Name = nodeName;
                boneTrack->BoneName = nodeName;

                for (uint32_t k = 0; k < channel->mNumPositionKeys; ++k)
                {
                    auto& key = channel->mPositionKeys[k];
                    boneTrack->PositionKeys.push_back({
                        static_cast<float>(key.mTime / anim->mTicksPerSecond),
                        ToVector3f(key.mValue)
                    });
                }
                for (uint32_t k = 0; k < channel->mNumRotationKeys; ++k)
                {
                    auto& key = channel->mRotationKeys[k];
                    boneTrack->RotationKeys.push_back({
                        static_cast<float>(key.mTime / anim->mTicksPerSecond),
                        ToQuat(key.mValue)
                    });
                }
                for (uint32_t k = 0; k < channel->mNumScalingKeys; ++k)
                {
                    auto& key = channel->mScalingKeys[k];
                    boneTrack->ScaleKeys.push_back({
                        static_cast<float>(key.mTime / anim->mTicksPerSecond),
                        ToVector3f(key.mValue)
                    });
                }

                tracks.push_back(boneTrack);
            }

            if (!tracks.empty())
            {
                auto clip = AnimationClip::StaticCreate(animName, duration, fps, std::move(tracks));
                clips.push_back(clip);
            }
        }
        return clips;
    }

    // -----------------------------------------------------------------------
    // Node processing
    // -----------------------------------------------------------------------
    static void ProcessNode(
        const aiNode* aiNode,
        ObjectPtr<Node> parentNode,
        RCPtr<Prefab> prefab,
        const aiScene* scene,
        AssimpImporterSettings* settings,
        const string& meshFolder,
        array_list<RCPtr<AssetObject>>& importedAssets,
        RCPtr<Skeleton> skeleton)
    {
        auto newNode = prefab->NewNode(aiNode->mName.C_Str(), parentNode);

        // Apply local transform
        {
            Vector3f t, s;
            Quat4f r;
            DecomposeTRS(aiNode->mTransformation, t, r, s);
            auto transform = newNode->GetTransform();
            transform->SetPosition(t);
            transform->SetRotation(r);
            transform->SetScale(s);
        }

        // Process meshes attached to this node
        for (uint32_t i = 0; i < aiNode->mNumMeshes; ++i)
        {
            uint32_t meshIndex = aiNode->mMeshes[i];
            aiMesh* mesh = scene->mMeshes[meshIndex];
            string meshName = mesh->mName.length > 0 ? string(mesh->mName.C_Str()) : string(aiNode->mName.C_Str());

            bool hasBones = mesh->HasBones();
            if (hasBones && skeleton && !settings->SkinnedMeshToStaticMesh)
            {
                if (auto skinnedMesh = ProcessSkinnedMesh(scene, mesh, meshName, skeleton, settings->RecomputeTangents, settings->UseMikktspace))
                {
                    const auto meshPath = AssetDatabase::GetUniquePath(meshFolder + "/" + skinnedMesh->GetName());
                    AssetDatabase::CreateAsset(skinnedMesh, meshPath);
                    auto renderer = newNode->AddComponent<SkinnedMeshRendererComponent>();
                    renderer->SetSkinnedMesh(skinnedMesh);
                    ApplyMaterialPolicy(skinnedMesh->GetMaterialCount(), skinnedMesh->GetMaterialNames(), settings, meshFolder, importedAssets,
                        [renderer](int idx, RCPtr<Material> mat) { renderer->SetMaterial(idx, mat); });
                    importedAssets.push_back(skinnedMesh);
                }
            }
            else
            {
                if (auto staticMesh = ProcessStaticMesh(scene, mesh, meshName, settings->RecomputeTangents, settings->UseMikktspace))
                {
                    const auto meshPath = AssetDatabase::GetUniquePath(meshFolder + "/" + staticMesh->GetName());
                    AssetDatabase::CreateAsset(staticMesh, meshPath);
                    auto renderer = newNode->AddComponent<StaticMeshRendererComponent>();
                    renderer->SetStaticMesh(staticMesh);
                    ApplyMaterialPolicy(staticMesh->GetMaterialCount(), staticMesh->GetMaterialNames(), settings, meshFolder, importedAssets,
                        [renderer](int idx, RCPtr<Material> mat) { renderer->SetMaterial(idx, mat); });
                    importedAssets.push_back(staticMesh);
                }
            }
        }

        // Recurse children
        for (uint32_t i = 0; i < aiNode->mNumChildren; ++i)
        {
            ProcessNode(aiNode->mChildren[i], newNode, prefab, scene, settings, meshFolder, importedAssets, skeleton);
        }
    }

    // -----------------------------------------------------------------------
    // Main Import
    // -----------------------------------------------------------------------
    array_list<RCPtr<AssetObject>> AssimpImporter::Import(AssetImporterSettings* settings)
    {
        AssimpImporterSettings* aiSettings = static_cast<AssimpImporterSettings*>(settings);
        array_list<RCPtr<AssetObject>> importedAssets;

        for (auto& importFile : *settings->ImportFiles)
        {
            const string filepath = importFile.string();
            const string filename = PathUtil::GetFilenameWithoutExt(importFile.filename().string());
            const string targetMeshFolder = settings->ImportingTargetFolder;

            Assimp::Importer importer;
            importer.SetPropertyBool(AI_CONFIG_IMPORT_FBX_PRESERVE_PIVOTS, false);

            unsigned int importFlags =
                aiProcess_Triangulate |
                aiProcess_GenSmoothNormals |
                aiProcess_JoinIdenticalVertices |
                aiProcess_LimitBoneWeights |
                aiProcess_SortByPType |
                aiProcess_ImproveCacheLocality;

            if (aiSettings->RecomputeTangents)
            {
                importFlags |= aiProcess_CalcTangentSpace;
            }

            // Pulsar uses left-handed coordinate system (DirectX-style)
            importFlags |= aiProcess_ConvertToLeftHanded;

            const aiScene* scene = importer.ReadFile(filepath, importFlags);
            if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE)
            {
                Logger::Log(StringUtil::Concat("AssimpImporter failed to load '", filepath, "': ", importer.GetErrorString()));
                continue;
            }

            // Build Skeleton first (needed for skinned meshes and animation)
            RCPtr<Skeleton> skeleton = BuildSkeleton(scene, filename + "Skeleton");
            if (skeleton)
            {
                const auto skeletonPath = AssetDatabase::GetUniquePath(targetMeshFolder + "/" + skeleton->GetName());
                AssetDatabase::CreateAsset(skeleton, skeletonPath);
                importedAssets.push_back(skeleton);
            }

            // Build Prefab
            auto prefab = Prefab::StaticCreate(filename + "Prefab");
            auto prefabRoot = prefab->NewNode(filename, nullptr);

            // Apply root node transform to prefab root
            {
                Vector3f t, s;
                Quat4f r;
                DecomposeTRS(scene->mRootNode->mTransformation, t, r, s);
                prefabRoot->GetTransform()->SetPosition(t);
                prefabRoot->GetTransform()->SetRotation(r);
                prefabRoot->GetTransform()->SetScale(s);
            }

            for (uint32_t i = 0; i < scene->mRootNode->mNumChildren; ++i)
            {
                ProcessNode(scene->mRootNode->mChildren[i], prefabRoot, prefab, scene, aiSettings, targetMeshFolder, importedAssets, skeleton);
            }

            // Post-process: bind SkinnedMeshRendererComponents to skeleton root
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
                Logger::Log(StringUtil::Concat("AssimpImporter: Bound ", std::to_string(skinnedRendererCount), " SkinnedMeshRenderer(s) to skeleton root"));
            }

            // Build AnimationClips
            if (aiSettings->ImportAnimations && skeleton)
            {
                auto clips = BuildAnimationClips(scene, skeleton, filename);
                for (auto& clip : clips)
                {
                    const auto clipPath = AssetDatabase::GetUniquePath(targetMeshFolder + "/" + clip->GetName());
                    AssetDatabase::CreateAsset(clip, clipPath);
                    importedAssets.push_back(clip);
                }
            }

            AssetDatabase::CreateAsset(prefab, AssetDatabase::GetUniquePath(targetMeshFolder + "/" + filename + "Prefab"));
            importedAssets.push_back(prefab);
        }

        return importedAssets;
    }

} // namespace pulsared
