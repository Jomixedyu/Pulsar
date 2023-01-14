#include "FBXImporter.h"
#include <Apatite/Assets/StaticMesh.h>
#include <Apatite/Assets/Material.h>
#include <Apatite/Components/MeshContainerComponent.h>
#include <ThirdParty/assimp/Importer.hpp>
#include <ThirdParty/assimp/scene.h>
#include <ThirdParty/assimp/postprocess.h>
#include <Apatite/Components/StaticMeshRendererComponent.h>

namespace apatiteed
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
    static inline LinearColorf _Color4(const aiColor4D& v)
    {
        return { v.r, v.g, v.b, v.a };
    }
    static inline Vector3f _Vec3(const aiVector3D& v3)
    {
        return { v3.x, v3.y, v3.z };
    }
    static inline Vector2f _Vec2(aiVector2D v2)
    {
        return { v2.x, v2.y };
    }

    static StaticMesh_sp ProcessMesh(aiMesh* mesh, const aiScene* scene, const string& dir, float scale_factor)
    {
        StaticMeshVertexDataArray* vert_arr = new StaticMeshVertexDataArray;
        vert_arr->reserve(mesh->mNumVertices);

        for (unsigned int vertnum = 0; vertnum < mesh->mNumVertices; ++vertnum)
        {
            StaticMeshVertexData vert;
            vert.Position = _Vec3(mesh->mVertices[vertnum]);
            vert.Normal = _Vec3(mesh->mNormals[vertnum]);
            if (mesh->mTangents)
            {
                vert.Tangent = _Vec3(mesh->mTangents[vertnum]);
            }
            if (mesh->mBitangents)
            {
                vert.BitTangent = _Vec3(mesh->mBitangents[vertnum]);
            }

            for (size_t i = 0; i < APATITE_STATICMESH_MAX_TEXTURE_COORDS; i++)
            {
                if (mesh->HasTextureCoords(i))
                {
                    auto v = mesh->mTextureCoords[i][vertnum];
                    vert.Coords[i] = { v.x, v.y };
                }
            }

            if (mesh->HasVertexColors(0))
            {
                vert.VertColor = _Color4(mesh->mColors[0][vertnum]);
            }

            vert_arr->push_back(std::move(vert));
        }
        array_list<uint32_t> indices;
        indices.reserve(mesh->mNumFaces);

        for (unsigned int i = 0; i < mesh->mNumFaces; i++)
        {
            aiFace face = mesh->mFaces[i];
            for (unsigned int j = 0; j < face.mNumIndices; j++)
            {
                indices.push_back(face.mIndices[j]);
            }
        }

        //
        //if (mesh->mMaterialIndex >= 0)
        //{
        //    aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];
        //    array_list<Texture2D*> diffuseMaps = loadMaterialTextures(material,
        //        aiTextureType_DIFFUSE, "mat_diffuse_tex", dir);
        //    textures->insert(textures->end(), diffuseMaps.begin(), diffuseMaps.end());

        //    array_list<Texture2D*> specularMaps = loadMaterialTextures(material,
        //        aiTextureType_SPECULAR, "mat_specular_tex", dir);
        //    textures->insert(textures->end(), specularMaps.begin(), specularMaps.end());

        //    array_list<Texture2D*> normalMaps = loadMaterialTextures(material,
        //        aiTextureType_NORMALS, "mat_normal_tex", dir);
        //    textures->insert(textures->end(), normalMaps.begin(), normalMaps.end());
        //}


        StaticMesh_sp static_mesh = StaticMesh::StaticCreate(std::move(mksptr(vert_arr)), std::move(indices));
        static_mesh->set_name(mesh->mName.C_Str());
        static_mesh->BindGPU();
        return static_mesh;
    }

    static void ProcessNode(aiNode* node, const aiScene* scene, Node_sp pnode, const string& dir, float scale_factor)
    {
        for (unsigned int i = 0; i < node->mNumMeshes; i++)
        {
            aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];

            Node_sp n = Node::StaticCreate(mesh->mName.C_Str());
            n->set_parent(pnode);

            n->AddComponent<MeshContainerComponent>()->set_mesh(ProcessMesh(mesh, scene, dir, scale_factor));
            n->AddComponent<StaticMeshRendererComponent>();
        }
        for (unsigned int i = 0; i < node->mNumChildren; i++)
        {
            ProcessNode(node->mChildren[i], scene, pnode, dir, scale_factor);
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

        ProcessNode(scene->mRootNode, scene, node, directory, scale_factor);

        return node;
    }
}