#include "Importers/AssimpFBXImporter.h"

#include "Pulsar/Scene.h"

#include <Pulsar/Application.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Assets/StaticMesh.h>
#include <Pulsar/Components/RendererComponent.h>
#include <Pulsar/Components/StaticMeshRendererComponent.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>

namespace pulsared
{

    //static array_list<Texture2D*> loadMaterialTextures(aiMaterial* mat, aiTextureType type, string typeName, const string& dir)
//{
//    array_list<Texture2D*> textures;
//    auto d = mat->GetTextureCount(aiTextureType::aiTextureType_HEIGHT);
//    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
//    {
//        aiString str;
//        mat->GetTexture(type, i, &str);
//        Texture2D* texture = new Texture2D;

//        auto name = StringUtil::Concat(dir, "/", PathUtil::GetFilename(str.C_Str()));

//        //TODO:
//        //texture->SetData(str.C_Str(), typeName, Resource::Load<Bitmap>(name, true));
//        //Texture2D* texture = Resource::Load<Texture2D>(name);


//        //texture.id = TextureFromFile(str.C_Str(), directory);
//        //texture.type = typeName;
//        //texture.path = str;
//        textures.push_back(texture);
//    }
//    return textures;
//}
    static inline auto ToColor(const aiColor4D& v)
    {
        return Color4b(uint8_t(v.r * 255), uint8_t(v.g * 255), uint8_t(v.b * 255), uint8_t(v.a * 255));
    }
    static inline Vector3f ToVector3f(const aiVector3D& v3)
    {
        return { v3.x, v3.y, v3.z };
    }
    static inline Vector2f ToVector2f(aiVector2D v2)
    {
        return { v2.x, v2.y };
    }

    static void _ProcessMesh(StaticMeshSection* section, aiMesh* mesh, const aiScene* scene)
    {
        const unsigned int vertCount = mesh->mNumVertices;

        // 确定实际 UV 套数
        uint8_t numUV = 0;
        for (int i = 0; i < STATICMESH_MAX_TEXTURE_COORDS; i++)
        {
            if (mesh->HasTextureCoords(i)) numUV = (uint8_t)(i + 1);
        }
        section->NumTexCoords = numUV;

        // Position（必须有）
        section->Positions.resize(vertCount);
        for (unsigned int i = 0; i < vertCount; i++)
            section->Positions[i] = ToVector3f(mesh->mVertices[i]);

        // Normal
        if (mesh->mNormals)
        {
            section->Normals.resize(vertCount);
            for (unsigned int i = 0; i < vertCount; i++)
                section->Normals[i] = ToVector3f(mesh->mNormals[i]);
        }

        // Tangent（Bitangent 不存，运行时由 Normal×Tangent 计算）
        if (mesh->mTangents)
        {
            section->Tangents.resize(vertCount);
            for (unsigned int i = 0; i < vertCount; i++)
                section->Tangents[i] = ToVector3f(mesh->mTangents[i]);
        }

        // VertexColor
        if (mesh->HasVertexColors(0))
        {
            section->Colors.resize(vertCount);
            for (unsigned int i = 0; i < vertCount; i++)
                section->Colors[i] = ToColor(mesh->mColors[0][i]);
        }

        // TexCoords（只存实际有的套数）
        section->TexCoords.resize(numUV);
        for (uint8_t uvIdx = 0; uvIdx < numUV; uvIdx++)
        {
            section->TexCoords[uvIdx].resize(vertCount);
            for (unsigned int i = 0; i < vertCount; i++)
            {
                auto& v = mesh->mTextureCoords[uvIdx][i];
                section->TexCoords[uvIdx][i] = { v.x, v.y };
            }
        }

        // Indices
        section->Indices.reserve(mesh->mNumFaces * 3);
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
                section->Indices.push_back(face.mIndices[j]);
        }
    }

    static void _ProcessNode(aiNode* node, const aiScene* scene, ObjectPtr<Node> pnode, Scene_ref pscene, const string& dir, float scale_factor)
    {
        array_list<StaticMeshSection> sections;
        sections.resize(node->mNumMeshes);

        array_list<RCPtr<Material>> materials;
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            _ProcessMesh(&sections[i], mesh, scene);
            auto sectionMat = scene->mMaterials[mesh->mMaterialIndex];

            materials.push_back(Material::StaticCreate(nullptr, sectionMat->GetName().C_Str()));

            sections[i].MaterialIndex = materials.size() - 1;
        }
        
        
        auto newNode = pscene->NewNode(node->mName.C_Str(), pnode);

        array_list<string> materialNames;
        for (auto& mat : materials)
        {
            materialNames.push_back(mat->GetName());
        }

        if (node->mNumMeshes > 0)
        {
            auto staticMesh = StaticMesh::StaticCreate(node->mName.C_Str(), std::move(sections), std::move(materialNames));

            auto renderer = newNode->AddComponent<StaticMeshRendererComponent>();
            for (auto& mat : materials)
            {
                renderer->AddMaterial(mat);
            }
        }

        // process children
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            _ProcessNode(node->mChildren[i], scene, newNode, pscene, dir, scale_factor);
        }
    }

    ObjectPtr<Node> AssimpFBXImporter::Import(string_view path, string& error)
    {
        // float scale_factor = 1;
        //
        // auto node = Node::StaticCreate(PathUtil::GetFilenameWithoutExt(path));
        //
        // Assimp::Importer importer;
        // const aiScene* scene = importer.ReadFile(path.data(), aiProcess_Triangulate | aiProcess_FlipUVs);
        //
        // if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        // {
        //     error = importer.GetErrorString();
        //     return nullptr;
        // }
        // string directory = string{ path.substr(0, path.find_last_of('/')) };
        //
        // for (unsigned int i = 0; i < scene->mRootNode->mNumChildren; i++)
        // {
        //     _ProcessNode(scene->mRootNode->mChildren[i], scene, node, directory, scale_factor);
        // }
        //
        // return node;
        return {};
    }
}