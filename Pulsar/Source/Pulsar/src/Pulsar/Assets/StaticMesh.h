#pragma once

#include "Mesh.h"
#include <Pulsar/IGPUResource.h>
#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/Material.h>
#include "Pulsar/Rendering/PrimitiveStruct.h"

namespace pulsar
{
    class Texture2D;

    using MeshIndicesType = uint32_t;

    // 序列化层：按属性分离存储，与 GPU Buffer 布局无关
    struct StaticMeshSection
    {
        uint8_t                          NumTexCoords = 0; // 实际使用的 UV 套数
        array_list<Vector3f>             Positions;
        array_list<Vector3f>             Normals;
        array_list<Vector4f>             Tangents;  // xyz=切线方向，w=副切线符号(+1/-1)
        array_list<Color4b>              Colors;
        array_list<array_list<Vector2f>> TexCoords;   // [uvIndex][vertexIndex]
        array_list<MeshIndicesType>      Indices;
        int32_t                          MaterialIndex = 0;

        size_t GetIndicesAllocSize() const
        {
            return Indices.size() * sizeof(MeshIndicesType);
        }

        // 构建 GPU 上传用的临时交错 Buffer
        array_list<StaticMeshVertex> BuildInterleavedVertices() const;
    };

    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, StaticMeshSection& data);


    class StaticMesh : public Mesh
    {
        friend class StaticMeshAssetSerializer;
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::StaticMesh, Mesh)
        CORELIB_CLASS_ATTR(new AssetIconAttribute("Editor/Icons/staticmesh.png"))
    public:
        constexpr static int32_t SerializeVersion = 2;
        StaticMesh() = default;
        ~StaticMesh() override;
    public:
        static gfx::GFXVertexLayoutDescription_sp StaticGetVertexLayout();

        virtual void Serialize(AssetSerializer* s) override;

        static RCPtr<StaticMesh> StaticCreate(
            string_view name,
            array_list<StaticMeshSection>&& vertData,
            array_list<string>&& materialNames);

        void CalcBounds();
    protected:
        virtual void OnInstantiateAsset(AssetObject* obj) override;
    public:
        // Override
        virtual size_t GetVertexCount() override { return 0; }

        StaticMeshSection& GetMeshSection(int i) { return m_sections[i]; }
        size_t GetMeshSectionCount() const { return m_sections.size(); }
        const array_list<string>& GetMaterialNames() const { return m_materialNames; }
        size_t GetMaterialCount() const { return m_materialNames.size(); }

        BoxSphereBounds3f GetBounds() const { return m_bounds; }
    public:
        bool CreateGPUResource() override;
        void DestroyGPUResource() override;
        bool IsCreatedGPUResource() const override;
        const array_list<gfx::GFXBuffer_sp>& GetGPUResourceVertexBuffers() const { return m_vertexBuffers; }
        const array_list<gfx::GFXBuffer_sp>& GetGPUResourceIndicesBuffers() const { return m_indicesBuffers; }
    protected: // serialization data
        array_list<StaticMeshSection> m_sections;
        array_list<string> m_materialNames;
    protected: // runtime data
        bool m_isCreatedResource = false;
        array_list<gfx::GFXBuffer_sp> m_vertexBuffers;
        array_list<gfx::GFXBuffer_sp> m_indicesBuffers;

        BoxSphereBounds3f m_bounds{};
    };
    DECL_PTR(StaticMesh);


}
