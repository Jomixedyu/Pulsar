#pragma once

#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/IBindGPU.h>
#include "Mesh.h"

namespace pulsar
{
    class Texture2D;

    constexpr int STATICMESH_MAX_TEXTURE_COORDS = 4;

    //struct StaticMeshSectionSerializeData
    //{
    //    array_list<Vector3f> Position;
    //    array_list<Vector3f> Normal;
    //    array_list<Vector3f> Tangent;
    //    array_list<Vector3f> Bitangent;
    //    array_list<uint32_t> Indices;
    //    array_list<Color8b4> Color;
    //    array_list<Vector2f> TexCoords[STATICMESH_MAX_TEXTURE_COORDS];
    //    int32_t              TexCoordsCount;
    //    int32_t              MaterialIndex;
    //};

    //struct StaticMeshSerializeData
    //{
    //    array_list<StaticMeshSectionSerializeData> Sections;
    //    array_list<ObjectPtr<Material>>            Materials;
    //};

    //ser::Stream& ReadWriteStream(ser::Stream& stream, bool isWrite, StaticMeshSectionSerializeData& data);
    //ser::Stream& ReadWriteStream(ser::Stream& stream, bool isWrite, StaticMeshSerializeData& data);

    struct StaticMeshVertex
    {
        Vector3f Position;
        Vector3f Normal;
        Vector3f Tangent;
        Vector3f Bitangent;
        Color8b4 Color;
        Vector2f TexCoords[STATICMESH_MAX_TEXTURE_COORDS];
    };


    struct StaticMeshSection
    {
        array_list<StaticMeshVertex> Vertex;
        array_list<uint32_t>         Indices;
        int32_t                      MaterialIndex;
    };

    ser::Stream& ReadWriteStream(ser::Stream& stream, bool isWrite, StaticMeshVertex& data);
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool isWrite, StaticMeshSection& data);


    class StaticMesh : public Mesh
    {
        friend class StaticMeshAssetSerializer;
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::StaticMesh, Mesh)
    public:
        constexpr static int32_t SerializeVersion = 1;
        StaticMesh() {}
        ~StaticMesh();
    public:
        virtual void OnSerialize(AssetSerializer* serializer) override;

        static sptr<StaticMesh> StaticCreate(array_list<StaticMeshSection>&& vertData);
    protected:
        virtual void OnInstantiateAsset(sptr<AssetObject>& obj) override;
    public:

    public:

    public:

    public:
        // Override
        virtual size_t GetVertexCount() override { return 0; }


    protected:
        //array_list<StaticMeshSectionSerializeData> m_sections;
        array_list<StaticMeshSection> m_sections;
    };
    CORELIB_DECL_SHORTSPTR(StaticMesh);


}
