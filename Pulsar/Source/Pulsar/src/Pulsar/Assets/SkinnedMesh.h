#pragma once

#include "StaticMesh.h"
#include "Skeleton.h"
#include <Pulsar/Rendering/PrimitiveStruct.h>

namespace pulsar
{
    // 序列化层：在 StaticMeshSection 基础上增加蒙皮数据
    struct SkinnedMeshSection
    {
        uint8_t                          NumTexCoords = 0;
        array_list<Vector3f>             Positions;
        array_list<Vector3f>             Normals;
        array_list<Vector4f>             Tangents;
        array_list<Color4b>              Colors;
        array_list<array_list<Vector2f>> TexCoords;         // [uvIndex][vertexIndex]
        array_list<MeshIndicesType>      Indices;
        int32_t                          MaterialIndex = 0;

        // 蒙皮数据（每顶点 4 个骨骼影响）
        array_list<std::array<uint32_t, SKINNEDMESH_MAX_BONE_INFLUENCES>> BoneIndices;
        array_list<std::array<float,    SKINNEDMESH_MAX_BONE_INFLUENCES>> BoneWeights;

        size_t GetIndicesAllocSize() const { return Indices.size() * sizeof(MeshIndicesType); }

        // 构建 GPU 上传用的临时交错 Buffer
        array_list<SkinnedMeshVertex> BuildInterleavedVertices() const;
    };

    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, SkinnedMeshSection& data);

    class SkinnedMesh : public Mesh
    {
        friend class SkinnedMeshAssetSerializer;
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SkinnedMesh, Mesh);
        CORELIB_CLASS_ATTR(new AssetIconAttribute("Editor/Icons/skinnedmesh.png"))
    public:
        constexpr static int32_t SerializeVersion = 2; // v2: 独立 Skeleton 引用
        SkinnedMesh() = default;
        ~SkinnedMesh() override;
    public:
        static gfx::GFXVertexLayoutDescription_sp StaticGetVertexLayout();

        virtual void Serialize(AssetSerializer* s) override;

        static RCPtr<SkinnedMesh> StaticCreate(
            string_view                      name,
            RCPtr<Skeleton>                  skeleton,
            array_list<SkinnedMeshSection>&& sections,
            array_list<string>&&             materialNames);

        // IGPUResource
        size_t GetVertexCount() override { return 0; }
        bool   IsCreatedGPUResource() const override;
        bool   CreateGPUResource() override;
        void   DestroyGPUResource() override;

        // Skeleton 访问（通过 Skeleton 资产获取骨骼层级）
        RCPtr<Skeleton>    GetSkeleton()    const { return m_skeleton; }
        void               SetSkeleton(RCPtr<Skeleton> sk) { m_skeleton = sk; }

        SkinnedMeshSection&                  GetMeshSection(int i)       { return m_sections[i]; }
        size_t                               GetMeshSectionCount()  const { return m_sections.size(); }
        const array_list<string>&            GetMaterialNames()     const { return m_materialNames; }
        size_t                               GetMaterialCount()     const { return m_materialNames.size(); }

        const array_list<gfx::GFXBuffer_sp>& GetGPUResourceVertexBuffers()  const { return m_vertexBuffers; }
        const array_list<gfx::GFXBuffer_sp>& GetGPUResourceIndicesBuffers() const { return m_indicesBuffers; }

    protected:
        void OnCollectAssetDependencies(array_list<jxcorlib::guid_t>& deps) override;
        virtual void OnInstantiateAsset(AssetObject* obj) override;

    protected: // serialization data
        RCPtr<Skeleton>               m_skeleton;       // 骨骼层级（独立资产）
        array_list<SkinnedMeshSection> m_sections;
        array_list<string>            m_materialNames;

    protected: // runtime data
        bool m_isCreatedResource = false;
        array_list<gfx::GFXBuffer_sp> m_vertexBuffers;
        array_list<gfx::GFXBuffer_sp> m_indicesBuffers;
    };
    DECL_PTR(SkinnedMesh);

} // namespace pulsar
