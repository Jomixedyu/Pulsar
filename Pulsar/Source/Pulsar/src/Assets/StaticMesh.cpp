#include "Assets/StaticMesh.h"
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Assets/Texture2D.h>

namespace pulsar
{
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, StaticMeshVertexBuildData& vert)
    {
        math::ReadWriteStream(stream, is_write, vert.Position);
        math::ReadWriteStream(stream, is_write, vert.Normal);
        math::ReadWriteStream(stream, is_write, vert.Tangent);
        math::ReadWriteStream(stream, is_write, vert.BitTangent);
        for (size_t i = 0; i < APATITE_STATICMESH_MAX_TEXTURE_COORDS; i++)
        {
            math::ReadWriteStream(stream, is_write, vert.Coords[i]);
        }
        math::ReadWriteStream(stream, is_write, vert.VertColor);

        return stream;
    }


    void StaticMesh::SerializeBuildData(ser::Stream& stream, bool is_ser)
    {
        int32_t ser_ver;
        if (is_ser) ser_ver = SerializeVersion;

        ReadWriteStream(stream, is_ser, ser_ver);
        assert(!is_ser && ser_ver == SerializeVersion);

        if (!is_ser)
        {
            this->m_rawData = mksptr(new StaticMeshVertexBuildDataArray);
        }
        ReadWriteStream(stream, is_ser, *this->m_rawData);
    }

    void StaticMesh::OnInstantiateAsset(sptr<AssetObject>& obj)
    {
        auto mesh = sptr_cast<StaticMesh>(obj);
        assert(mesh);
        mesh->m_rawData = this->m_rawData;
    }

    StaticMesh::~StaticMesh()
    {

    }

    StaticMesh_sp StaticMesh::StaticCreate(sptr<StaticMeshVertexBuildDataArray>&& managed_data, array_list<uint32_t>&& indices_data)
    {
        auto mesh = mksptr(new StaticMesh);
        mesh->Construct();
        mesh->m_rawData = std::move(managed_data);
        mesh->indices = std::move(indices_data);
        
        return mesh;
    }


}