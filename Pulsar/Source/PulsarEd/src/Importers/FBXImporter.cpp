#include "Importers/FBXImporter.h"
#include <Pulsar/Assets/StaticMesh.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Components/MeshContainerComponent.h>
#include <ThirdParty/assimp/Importer.hpp>
#include <ThirdParty/assimp/scene.h>
#include <ThirdParty/assimp/postprocess.h>
#include <Pulsar/Components/StaticMeshRendererComponent.h>
#include <Pulsar/Application.h>

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
    static inline Color8b4 _Color4(const aiColor4D& v)
    {
        return Color8b4(v.r * 255, v.g * 255, v.b * 255, v.a * 255);
    }
    static inline Vector3f _Vec3(const aiVector3D& v3)
    {
        return { v3.x, v3.y, v3.z };
    }
    static inline Vector2f _Vec2(aiVector2D v2)
    {
        return { v2.x, v2.y };
    }

    static void _ProcessMesh(StaticMeshSection* section, aiMesh* mesh, const aiScene* scene)
    {
        section->Vertex.resize(mesh->mNumVertices);
        for (unsigned int vertnum = 0; vertnum < mesh->mNumVertices; ++vertnum)
        {
            StaticMeshVertex& vert = section->Vertex[vertnum];
            vert.Position = _Vec3(mesh->mVertices[vertnum]);
            vert.Normal = _Vec3(mesh->mNormals[vertnum]);
            if (mesh->mTangents)
            {
                vert.Tangent = _Vec3(mesh->mTangents[vertnum]);
            }
            if (mesh->mBitangents)
            {
                vert.Bitangent = _Vec3(mesh->mBitangents[vertnum]);
            }

            for (size_t i = 0; i < STATICMESH_MAX_TEXTURE_COORDS; i++)
            {
                if (mesh->HasTextureCoords(i))
                {
                    auto v = mesh->mTextureCoords[i][vertnum];
                    vert.TexCoords[i] = { v.x, v.y };
                }
            }

            if (mesh->HasVertexColors(0))
            {
                vert.Color = _Color4(mesh->mColors[0][vertnum]);
            }

        }

        section->Indices.reserve(mesh->mNumFaces);
        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                section->Indices.push_back(face.mIndices[j]);
            }
        }

    }

    static void _ProcessNode(aiNode* node, const aiScene* scene, Node_sp pnode, const string& dir, float scale_factor)
    {
        array_list<StaticMeshSection> sections;
        sections.resize(node->mNumMeshes);

        array_list<Material_sp> materials;
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
            _ProcessMesh(&sections[i], mesh, scene);
            auto sectionMat = scene->mMaterials[mesh->mMaterialIndex];

            materials.push_back(Material::StaticCreate(sectionMat->GetName().C_Str(), nullptr));

            sections[i].MaterialIndex = materials.size() - 1;
        }

        Node_sp n = Node::StaticCreate(node->mName.C_Str());
        n->set_parent(pnode);

        array_list<string> materialNames;
        for (auto& mat : materials)
        {
            materialNames.push_back(mat->GetName());
        }

        auto staticMesh = StaticMesh::StaticCreate(node->mName.C_Str(), std::move(sections), std::move(materialNames));

        n->AddComponent<MeshContainerComponent>()->SetMesh(staticMesh);

        auto renderer = n->AddComponent<StaticMeshRendererComponent>();
        for (auto& mat : materials)
        {
            renderer->GetMaterials()->Add(mat);
        }

        // process children
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            _ProcessNode(node->mChildren[i], scene, pnode, dir, scale_factor);
        }
    }

    Node_sp FBXImporter::Import(string_view path)
    {
        float scale_factor = 1;

        Node_sp node = Node::StaticCreate(PathUtil::GetFilenameWithoutExt(path));

        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(path.data(), aiProcess_Triangulate | aiProcess_FlipUVs);

        if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
        {
            throw importer.GetErrorString();
            return nullptr;
        }
        string directory = string{ path.substr(0, path.find_last_of('/')) };

        _ProcessNode(scene->mRootNode, scene, node, directory, scale_factor);

        return node;
    }
}