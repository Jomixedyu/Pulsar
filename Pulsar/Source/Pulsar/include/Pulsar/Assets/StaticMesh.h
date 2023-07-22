#pragma once

#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/IBindGPU.h>
#include "Mesh.h"

namespace pulsar
{
    class Texture2D;

    constexpr static int APATITE_STATICMESH_MAX_TEXTURE_COORDS = 4;

    //build vertex data
    struct StaticMeshVertexBuildData
    {
        Vector3f Position;
        Vector3f Normal;
        Vector3f Tangent;
        Vector3f BitTangent;
        LinearColorf VertColor;
        Vector2f Coords[APATITE_STATICMESH_MAX_TEXTURE_COORDS];
    };
    using StaticMeshVertexBuildDataArray = array_list<StaticMeshVertexBuildData>;

    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, StaticMeshVertexBuildData& vert);

    struct StaticMeshRawData
    {
        array_list<Vector3f> Positions;
        array_list<Vector3f> Normals;
        array_list<Vector3f> Tangent;
        array_list<uint32_t> Indices;
        array_list<LinearColorf> VertColors;
        array_list<Vector2f> TexCoords[APATITE_STATICMESH_MAX_TEXTURE_COORDS];
        uint8_t TexCoordsFlags;

        bool HasTexCoord(int index)
        {
            return TexCoordsFlags | (1 << index);
        }
    };

    class StaticMesh : public Mesh
    {
        friend class StaticMeshAssetSerializer;
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::StaticMesh, Mesh)
    public:
        constexpr static int32_t SerializeVersion = 1;
        StaticMesh() {}
        ~StaticMesh();
    public:
        static sptr<StaticMesh> StaticCreate(sptr<StaticMeshVertexBuildDataArray>&& managed_data, array_list<uint32_t>&& indices_data);

    public:
        //virtual void SerializeAsset(ser::Stream& stream, bool is_ser);
        virtual void SerializeBuildData(ser::Stream& stream, bool is_ser) override;
        virtual void OnInstantiateAsset(sptr<AssetObject>& obj) override;
    public:

    public:

    public:
        uint32_t GetRenderHandle() const { return this->render_handle_; }
        const sptr<StaticMeshVertexBuildDataArray>& GetRawData() const { return m_rawData; }
    public:
        // Override
        virtual size_t GetVertexCount() override { return this->m_rawData->size(); }
        size_t GetIndicesCount() { return this->indices.size(); }
    public:
        uint32_t ebo_buffer_ = 0;
    protected:
        sptr<StaticMeshVertexBuildDataArray> m_rawData = nullptr;
        array_list<uint32_t> indices;
        uint32_t render_handle_ = 0;
        uint32_t render_buffer_ = 0;

        array_list<sptr<Material>> m_materials;
    };
    CORELIB_DECL_SHORTSPTR(StaticMesh);


}
