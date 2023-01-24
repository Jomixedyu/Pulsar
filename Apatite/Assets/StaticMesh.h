#pragma once

#include <Apatite/AssetObject.h>
#include <Apatite/Math.h>
#include <Apatite/Assets/Material.h>
#include <Apatite/IBindGPU.h>
#include "Mesh.h"

namespace apatite
{
    class Texture2D;

    constexpr static int APATITE_STATICMESH_MAX_TEXTURE_COORDS = 8;

    //build vertex data
    struct StaticMeshVertexData
    {
        Vector3f Position;
        Vector3f Normal;
        Vector3f Tangent;
        Vector3f BitTangent;
        Vector2f Coords[APATITE_STATICMESH_MAX_TEXTURE_COORDS];
        LinearColorf VertColor;
    };
    using StaticMeshVertexDataArray = array_list<StaticMeshVertexData>;

    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, StaticMeshVertexData& vert);

    struct StaticMeshRawData
    {
        array_list<Vector3f> Positions;
        array_list<Vector3f> Normals;
        array_list<Vector3f> Tangent;
        array_list<uint32_t> Indices;
        array_list<LinearColorf> VertColors;
        array_list<Vector2f> TexCoords[APATITE_STATICMESH_MAX_TEXTURE_COORDS];
        uint8_t TexCoordsFlags;
    };

    class StaticMesh : public Mesh, public IBindGPU
    {
        friend class StaticMeshAssetSerializer;
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::StaticMesh, Mesh)
    public:
        constexpr static int32_t SerializeVersion = 1;
        StaticMesh() {}
        ~StaticMesh();
    public:
        static sptr<StaticMesh> StaticCreate(sptr<StaticMeshVertexDataArray>&& managed_data, array_list<uint32_t>&& indices_data);

    public:
        virtual void Serialize(ser::Stream& stream, bool is_ser) override;
        virtual void OnInstantiateAsset(sptr<AssetObject>& obj) override;
    public:

    public:
        // IBindGPU
        virtual void BindGPU() override;
        virtual void UnBindGPU() override;
        virtual bool GetIsBindGPU() override;
    public:
        uint32_t GetRenderHandle() const { return this->render_handle_; }
    public:
        // Override
        size_t GetVertexCount() override { return this->raw_data_->size(); }
    public:
        uint32_t ebo_buffer_ = 0;
    protected:
        sptr<StaticMeshVertexDataArray> raw_data_ = nullptr;
        array_list<uint32_t> indices;
        uint32_t render_handle_ = 0;
        uint32_t render_buffer_ = 0;
    };
    CORELIB_DECL_SHORTSPTR(StaticMesh);


}
