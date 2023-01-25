#include "StaticMesh.h"
#include <ThirdParty/glad/glad.h>
#include <Apatite/Assets/Shader.h>
#include <Apatite/Assets/Texture2D.h>

namespace apatite
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
            this->raw_data_ = mksptr(new StaticMeshVertexBuildDataArray);
        }
        ReadWriteStream(stream, is_ser, *this->raw_data_);


        if (!is_ser)
        {
            this->BindGPU();
        }
    }

    void StaticMesh::OnInstantiateAsset(sptr<AssetObject>& obj)
    {
        auto mesh = sptr_cast<StaticMesh>(obj);
        assert(mesh);
        mesh->raw_data_ = this->raw_data_;
    }

    StaticMesh::~StaticMesh()
    {
        this->UnBindGPU();
    }

    StaticMesh_sp StaticMesh::StaticCreate(sptr<StaticMeshVertexBuildDataArray>&& managed_data, array_list<uint32_t>&& indices_data)
    {
        auto mesh = mksptr(new StaticMesh);
        mesh->Construct();
        mesh->raw_data_ = managed_data;
        mesh->indices = std::move(indices_data);
        return mesh;
    }


    void StaticMesh::BindGPU()
    {
        assert(!this->GetIsBindGPU());

        uint32_t& vao = this->render_handle_;
        glGenVertexArrays(1, &vao);


        uint32_t& vbo = this->render_buffer_;
        glGenBuffers(1, &vbo);


        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        auto vertex_arr_size = this->raw_data_->size();
        glBufferData(GL_ARRAY_BUFFER, this->raw_data_->size() * sizeof(StaticMeshVertexBuildData), this->raw_data_->data(), GL_STATIC_DRAW);



        int attr_index = 0;

        glVertexAttribPointer(attr_index, decltype(StaticMeshVertexBuildData::Position)::row_count, GL_FLOAT, GL_FALSE, sizeof(StaticMeshVertexBuildData::Position), (void*)offsetof(StaticMeshVertexBuildData, Position));
        glEnableVertexAttribArray(attr_index);
        ++attr_index;

        glVertexAttribPointer(attr_index, decltype(StaticMeshVertexBuildData::Normal)::row_count, GL_FLOAT, GL_FALSE, sizeof(StaticMeshVertexBuildData::Normal), (void*)offsetof(StaticMeshVertexBuildData, Normal));
        glEnableVertexAttribArray(attr_index);
        ++attr_index;

        glVertexAttribPointer(attr_index, decltype(StaticMeshVertexBuildData::Tangent)::row_count, GL_FLOAT, GL_FALSE, sizeof(StaticMeshVertexBuildData::Tangent), (void*)offsetof(StaticMeshVertexBuildData, Tangent));
        glEnableVertexAttribArray(attr_index);
        ++attr_index;

        glVertexAttribPointer(attr_index, decltype(StaticMeshVertexBuildData::BitTangent)::row_count, GL_FLOAT, GL_FALSE, sizeof(StaticMeshVertexBuildData::BitTangent), (void*)offsetof(StaticMeshVertexBuildData, BitTangent));
        glEnableVertexAttribArray(attr_index);
        ++attr_index;

        glVertexAttribPointer(attr_index, 2, GL_FLOAT, GL_FALSE, 8, (void*)offsetof(StaticMeshVertexBuildData, Coords));
        glEnableVertexAttribArray(attr_index);
        ++attr_index;

        glVertexAttribPointer(attr_index, 4, GL_FLOAT, GL_FALSE, sizeof(StaticMeshVertexBuildData::VertColor), (void*)offsetof(StaticMeshVertexBuildData, VertColor));
        glEnableVertexAttribArray(attr_index);
        ++attr_index;

        uint32_t ebo;
        glGenBuffers(1, &ebo);
        this->ebo_buffer_ = ebo;
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint32_t) * this->indices.size(), this->indices.data(), GL_STATIC_DRAW);

        glBindVertexArray(GL_NONE);
        glBindBuffer(GL_ARRAY_BUFFER, GL_NONE);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GL_NONE);

    }

    void StaticMesh::UnBindGPU()
    {
        if (this->GetIsBindGPU())
        {
            glDeleteVertexArrays(1, &this->render_handle_);
            glDeleteBuffers(1, &this->render_buffer_);

            this->render_handle_ = 0;
            this->render_buffer_ = 0;
        }
    }

    bool StaticMesh::GetIsBindGPU()
    {
        return this->render_handle_ != 0;
    }

}