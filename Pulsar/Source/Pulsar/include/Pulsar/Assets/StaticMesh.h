#pragma once

#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/IBindGPU.h>
#include "Mesh.h"

namespace pulsar
{
    class Texture2D;

    constexpr int STATICMESH_MAX_TEXTURE_COORDS = 4;

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

        static sptr<StaticMesh> StaticCreate(
            string_view name,
            array_list<StaticMeshSection>&& vertData,
            array_list<string>&& materialNames);
    protected:
        virtual void OnInstantiateAsset(AssetObject_ref& obj) override;
    public:

    public:

    public:

    public:
        // Override
        virtual size_t GetVertexCount() override { return 0; }


    protected:
        //array_list<StaticMeshSectionSerializeData> m_sections;
        array_list<StaticMeshSection> m_sections;

        array_list<string> m_materialNames;
    };
    DECL_PTR(StaticMesh);


}
