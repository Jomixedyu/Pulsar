#pragma once

#include "Mesh.h"
#include <Pulsar/IGPUResource.h>
#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/Material.h>
#include "Pulsar/Rendering/PrimitiveStruct.h"

namespace pulsar
{
    class Texture2D;

    struct StaticMeshSection
    {
        array_list<StaticMeshVertex>  Vertex;
        array_list<uint32_t>          Indices;
        int32_t                       MaterialIndex;

        size_t GetVertexAllocSize() const
        {
            return Vertex.size() * sizeof(StaticMeshVertex);
        }
        size_t GetIndicesAllocSize() const
        {
            return Indices.size() * sizeof(uint32_t);
        }
    };



    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, StaticMeshVertex& data);
    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, StaticMeshSection& data);


    class StaticMesh : public Mesh
    {
        friend class StaticMeshAssetSerializer;
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::StaticMesh, Mesh)
    public:
        constexpr static int32_t SerializeVersion = 1;
        StaticMesh() = default;
        ~StaticMesh() override;
    public:
        static gfx::GFXVertexLayoutDescription_sp StaticGetVertexLayout();

        virtual void Serialize(AssetSerializer* s) override;

        static ObjectPtr<StaticMesh> StaticCreate(
            string_view name,
            array_list<StaticMeshSection>&& vertData,
            array_list<string>&& materialNames);
    protected:
        virtual void OnInstantiateAsset(AssetObject* obj) override;
    public:
        // Override
        virtual size_t GetVertexCount() override { return 0; }

        StaticMeshSection& GetMeshSection(int i) { return m_sections[i]; }
        size_t GetMeshSectionCount() const { return m_sections.size(); }
        const array_list<string>& GetMaterialNames() const { return m_materialNames; }
        size_t GetMaterialCount() const { return m_materialNames.size(); }
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
    };
    DECL_PTR(StaticMesh);


}
