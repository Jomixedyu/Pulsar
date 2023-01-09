#pragma once

#include <Apatite/AssetObject.h>
#include <Apatite/Math.h>
#include <Apatite/Assets/Material.h>
#include <Apatite/IBindGPU.h>

namespace apatite
{
    class Texture2D;

    constexpr static int APATITE_STATICMESH_MAX_TEXTURE_COORDS = 8;

    //runtime vertex data
    struct StaticMeshVertexData     //128
    {
        Vector3f Position;          //12
        Vector3f Normal;            //12
        Vector3f Tangent;           //12
        Vector3f BitTangent;        //12
        Vector2f Coords[APATITE_STATICMESH_MAX_TEXTURE_COORDS]; //8*n
        LinearColorf VertColor;     //16
    };
    using StaticMeshVertexDataArray = array_list<StaticMeshVertexData>;

    ser::Stream& ReadWriteStream(ser::Stream& stream, bool is_write, StaticMeshVertexData& vert);

    //struct StaticMeshRawData
    //{
    //    array_list<Vector3f> Positions;
    //    array_list<Vector3f> Normals;
    //    array_list<Vector3f> Tangent;
    //    array_list<uint32_t> Indices;
    //    array_list<LinearColorf> VertColors;
    //    array_list<Vector2f> TexCoords[APATITE_STATICMESH_MAX_TEXTURE_COORDS];
    //};

    class StaticMesh : public AssetObject, public IBindGPU
    {
        friend class StaticMeshAssetSerializer;
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::StaticMesh, AssetObject)
    public:
        constexpr static int32_t SerializeVersion = 1;
        StaticMesh() {}
        ~StaticMesh();
    public://static
        static sptr<StaticMesh> StaticCreate(sptr<StaticMeshVertexDataArray>&& managed_data, array_list<uint32_t>&& indices_data);

    public://member
        virtual void Serialize(ser::Stream& stream, bool is_ser) override;
        virtual void OnInstantiateAsset(sptr<AssetObject>& obj) override;
        virtual void Render();
    public:

    public:
        // IBindGPU
        virtual void BindGPU() override;
        virtual void UnBindGPU() override;
        virtual bool GetIsBindGPU() override;
    protected:
        sptr<StaticMeshVertexDataArray> raw_data_ = nullptr;
        array_list<uint32_t> indices;
        uint32_t render_handle_ = 0;
        uint32_t render_buffer_ = 0;
    };
    CORELIB_DECL_SHORTSPTR(StaticMesh);


}
