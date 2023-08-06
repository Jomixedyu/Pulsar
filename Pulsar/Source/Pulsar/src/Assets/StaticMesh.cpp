#include "Assets/StaticMesh.h"
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Assets/Texture2D.h>
#include "EngineMath.h"

namespace pulsar
{
    void StaticMesh::OnInstantiateAsset(sptr<AssetObject>& obj)
    {
        base::OnInstantiateAsset(obj);
        auto mesh = sptr_cast<StaticMesh>(obj);
        mesh->m_sections = m_sections;
    }

    StaticMesh::~StaticMesh()
    {

    }

    void StaticMesh::OnSerialize(AssetSerializer* serializer)
    {
        base::OnSerialize(serializer);

        ReadWriteStream(serializer->Stream, serializer->IsWrite, m_sections);
    }


    StaticMesh_sp StaticMesh::StaticCreate(
        string_view name,
        array_list<StaticMeshSection>&& vertData,
        array_list<string>&& materialNames)
    {
        StaticMesh_sp self = mksptr(new StaticMesh);
        self->Construct();
        self->name_ = name;
        self->m_sections = std::move(vertData);
        self->m_materialNames = std::move(materialNames);

        return self;
    }



    ser::Stream& ReadWriteStream(ser::Stream& stream, bool isWrite, StaticMeshVertex& data)
    {
        ReadWriteStream(stream, isWrite, data.Position);
        ReadWriteStream(stream, isWrite, data.Normal);
        ReadWriteStream(stream, isWrite, data.Tangent);
        ReadWriteStream(stream, isWrite, data.Bitangent);
        ReadWriteStream(stream, isWrite, data.Color);
        ReadWriteStream(stream, isWrite, data.TexCoords);
        return stream;
    }

    ser::Stream& ReadWriteStream(ser::Stream& stream, bool isWrite, StaticMeshSection& data)
    {
        ReadWriteStream(stream, isWrite, data.Vertex);
        ReadWriteStream(stream, isWrite, data.Indices);
        ReadWriteStream(stream, isWrite, data.MaterialIndex);
        return stream;
    }

}