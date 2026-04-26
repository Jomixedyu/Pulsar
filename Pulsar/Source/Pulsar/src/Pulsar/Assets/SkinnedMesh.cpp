#include "Assets/SkinnedMesh.h"

#include "Application.h"
#include "AssetSerializerUtil.h"
#include "EngineMath.h"
#include <Pulsar/AssetObject.h>

namespace pulsar
{
    // -----------------------------------------------------------------------
    // SkinnedMeshSection::BuildInterleavedVertices
    // -----------------------------------------------------------------------
    array_list<SkinnedMeshVertex> SkinnedMeshSection::BuildInterleavedVertices() const
    {
        const size_t vertCount = Positions.size();
        array_list<SkinnedMeshVertex> result(vertCount);

        for (size_t i = 0; i < vertCount; i++)
        {
            SkinnedMeshVertex& v = result[i];
            v.Position = Positions[i];
            v.Normal   = Normals.empty()  ? Vector3f{0, 1, 0}          : Normals[i];
            v.Tangent  = Tangents.empty() ? Vector4f{1, 0, 0, 1}       : Tangents[i];
            v.Color    = Colors.empty()   ? Color4b{255, 255, 255, 255} : Colors[i];

            for (uint8_t uvIdx = 0; uvIdx < NumTexCoords && uvIdx < STATICMESH_MAX_TEXTURE_COORDS; uvIdx++)
                v.TexCoords[uvIdx] = TexCoords[uvIdx][i];

            if (!BoneIndices.empty())
            {
                v.BoneIndices[0] = static_cast<uint8_t>(BoneIndices[i][0]);
                v.BoneIndices[1] = static_cast<uint8_t>(BoneIndices[i][1]);
                v.BoneIndices[2] = static_cast<uint8_t>(BoneIndices[i][2]);
                v.BoneIndices[3] = static_cast<uint8_t>(BoneIndices[i][3]);
                v.BoneWeights[0] = BoneWeights[i][0];
                v.BoneWeights[1] = BoneWeights[i][1];
                v.BoneWeights[2] = BoneWeights[i][2];
                v.BoneWeights[3] = BoneWeights[i][3];
            }
        }
        return result;
    }

    // -----------------------------------------------------------------------
    // Serialization helper for SkinnedMeshSection
    // -----------------------------------------------------------------------
    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, SkinnedMeshSection& data)
    {
        sser::ReadWriteStream(stream, isWrite, data.NumTexCoords);
        sser::ReadWriteStream(stream, isWrite, data.Positions);
        sser::ReadWriteStream(stream, isWrite, data.Normals);
        sser::ReadWriteStream(stream, isWrite, data.Tangents);
        sser::ReadWriteStream(stream, isWrite, data.Colors);
        sser::ReadWriteStream(stream, isWrite, data.TexCoords);
        sser::ReadWriteStream(stream, isWrite, data.Indices);
        sser::ReadWriteStream(stream, isWrite, data.MaterialIndex);

        // BoneIndices / BoneWeights 手动展开（sser vector 模板无法推导 std::array<T,N>）
        uint32_t vertCount = static_cast<uint32_t>(data.Positions.size());
        if (!isWrite)
        {
            data.BoneIndices.resize(vertCount);
            data.BoneWeights.resize(vertCount);
        }
        for (uint32_t vi = 0; vi < vertCount; ++vi)
            for (uint32_t bi = 0; bi < SKINNEDMESH_MAX_BONE_INFLUENCES; ++bi)
                sser::ReadWriteStream(stream, isWrite, data.BoneIndices[vi][bi]);
        for (uint32_t vi = 0; vi < vertCount; ++vi)
            for (uint32_t bi = 0; bi < SKINNEDMESH_MAX_BONE_INFLUENCES; ++bi)
                sser::ReadWriteStream(stream, isWrite, data.BoneWeights[vi][bi]);
        return stream;
    }

    // -----------------------------------------------------------------------
    // Vertex layout
    // -----------------------------------------------------------------------
    static auto _GetSkinnedVertexLayout()
    {
        auto layout = Application::GetGfxApp()->CreateVertexLayoutDescription();
        layout->BindingPoint = 0;
        layout->Stride = sizeof(SkinnedMeshVertex);

        layout->Attributes.push_back({(int)EngineInputSemantic::POSITION,  gfx::GFXVertexInputDataFormat::R32G32B32_SFloat,    offsetof(SkinnedMeshVertex, Position)});
        layout->Attributes.push_back({(int)EngineInputSemantic::NORMAL,    gfx::GFXVertexInputDataFormat::R32G32B32_SFloat,    offsetof(SkinnedMeshVertex, Normal)});
        layout->Attributes.push_back({(int)EngineInputSemantic::TANGENT,   gfx::GFXVertexInputDataFormat::R32G32B32A32_SFloat, offsetof(SkinnedMeshVertex, Tangent)});
        layout->Attributes.push_back({(int)EngineInputSemantic::COLOR,     gfx::GFXVertexInputDataFormat::R8G8B8A8_UNorm,     offsetof(SkinnedMeshVertex, Color)});

        layout->Attributes.push_back({(int)EngineInputSemantic::TEXCOORD0 + 0, gfx::GFXVertexInputDataFormat::R32G32_SFloat, offsetof(SkinnedMeshVertex, TexCoords[0])});
        layout->Attributes.push_back({(int)EngineInputSemantic::TEXCOORD0 + 1, gfx::GFXVertexInputDataFormat::R32G32_SFloat, offsetof(SkinnedMeshVertex, TexCoords[1])});
        layout->Attributes.push_back({(int)EngineInputSemantic::TEXCOORD0 + 2, gfx::GFXVertexInputDataFormat::R32G32_SFloat, offsetof(SkinnedMeshVertex, TexCoords[2])});
        layout->Attributes.push_back({(int)EngineInputSemantic::TEXCOORD0 + 3, gfx::GFXVertexInputDataFormat::R32G32_SFloat, offsetof(SkinnedMeshVertex, TexCoords[3])});
        layout->Attributes.push_back({(int)EngineInputSemantic::TEXCOORD0 + 4, gfx::GFXVertexInputDataFormat::R32G32_SFloat, offsetof(SkinnedMeshVertex, TexCoords[4])});
        layout->Attributes.push_back({(int)EngineInputSemantic::TEXCOORD0 + 5, gfx::GFXVertexInputDataFormat::R32G32_SFloat, offsetof(SkinnedMeshVertex, TexCoords[5])});
        layout->Attributes.push_back({(int)EngineInputSemantic::TEXCOORD0 + 6, gfx::GFXVertexInputDataFormat::R32G32_SFloat, offsetof(SkinnedMeshVertex, TexCoords[6])});
        layout->Attributes.push_back({(int)EngineInputSemantic::TEXCOORD0 + 7, gfx::GFXVertexInputDataFormat::R32G32_SFloat, offsetof(SkinnedMeshVertex, TexCoords[7])});

        layout->Attributes.push_back({(int)EngineInputSemantic::BLENDINDICES, gfx::GFXVertexInputDataFormat::R8G8B8A8_UInt,       offsetof(SkinnedMeshVertex, BoneIndices)});
        layout->Attributes.push_back({(int)EngineInputSemantic::BLENDWEIGHT,  gfx::GFXVertexInputDataFormat::R32G32B32A32_SFloat, offsetof(SkinnedMeshVertex, BoneWeights)});

        return layout;
    }

    gfx::GFXVertexLayoutDescription_sp SkinnedMesh::StaticGetVertexLayout()
    {
        static gfx::GFXVertexLayoutDescription_wp layout;
        if (layout.expired())
        {
            auto newLayout = _GetSkinnedVertexLayout();
            layout = newLayout;
            return newLayout;
        }
        return layout.lock();
    }

    // -----------------------------------------------------------------------
    // GPU Resource
    // -----------------------------------------------------------------------
    bool SkinnedMesh::CreateGPUResource()
    {
        if (m_isCreatedResource) return true;
        m_isCreatedResource = true;

        for (auto& section : m_sections)
        {
            auto verts = section.BuildInterleavedVertices();

            gfx::GFXBufferDesc vDesc{};
            vDesc.Usage       = gfx::GFXBufferUsage::Vertex;
            vDesc.StorageType = gfx::GFXBufferMemoryPosition::DeviceLocal;
            vDesc.BufferSize  = verts.size() * sizeof(SkinnedMeshVertex);
            vDesc.ElementSize = sizeof(SkinnedMeshVertex);
            auto vb = Application::GetGfxApp()->CreateBuffer(vDesc);
            vb->Fill(verts.data());
            m_vertexBuffers.push_back(vb);

            gfx::GFXBufferDesc iDesc{};
            iDesc.Usage       = gfx::GFXBufferUsage::Indices;
            iDesc.StorageType = gfx::GFXBufferMemoryPosition::DeviceLocal;
            iDesc.BufferSize  = section.GetIndicesAllocSize();
            iDesc.ElementSize = sizeof(MeshIndicesType);
            auto ib = Application::GetGfxApp()->CreateBuffer(iDesc);
            ib->Fill(section.Indices.data());
            m_indicesBuffers.push_back(ib);
        }
        return true;
    }

    void SkinnedMesh::DestroyGPUResource()
    {
        if (!m_isCreatedResource) return;
        m_isCreatedResource = false;
        m_vertexBuffers.clear();
        m_indicesBuffers.clear();
    }

    bool SkinnedMesh::IsCreatedGPUResource() const { return m_isCreatedResource; }

    // -----------------------------------------------------------------------
    // Serialize  （v2: SkeletonSerialize 以 GUID 引用方式存储）
    // -----------------------------------------------------------------------
    void SkinnedMesh::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        auto& stream  = s->GetStream();
        bool  isWrite = s->IsWrite;

        if (!isWrite)
        {
            m_sections.clear();
            m_materialNames.clear();
        }

        // --- Skeleton 引用（JSON 侧存 GUID 字符串）---
        if (s->IsWrite)
        {
            auto guidStr = s->Object->New(ser::VarientType::String);
            guidStr->Assign(m_skeleton ? m_skeleton->GetAssetGuid().to_string() : "");
            s->Object->Add("Skeleton", guidStr);
        }
        else
        {
            if (auto skObj = s->Object->At("Skeleton"))
            {
                auto guid = guid_t::parse(skObj->AsString());
                m_skeleton = RuntimeAssetManager::GetLoadedAssetByGuid<Skeleton>(guid);
            }
        }

        // --- Sections（二进制流）---
        uint32_t sectionCount = static_cast<uint32_t>(m_sections.size());
        sser::ReadWriteStream(stream, isWrite, sectionCount);
        if (!isWrite) m_sections.resize(sectionCount);
        for (auto& section : m_sections)
            ReadWriteStream(stream, isWrite, section);

        // --- MaterialNames（JSON 侧）---
        if (s->IsWrite)
        {
            auto arr = s->Object->New(ser::VarientType::Array);
            for (auto& name : m_materialNames) arr->Push(name);
            s->Object->Add("MaterialNames", arr);
        }
        else
        {
            if (auto arr = s->Object->At("MaterialNames"))
                for (int i = 0; i < arr->GetCount(); ++i)
                    m_materialNames.push_back(arr->At(i)->AsString());
        }
    }

    // -----------------------------------------------------------------------
    // StaticCreate
    // -----------------------------------------------------------------------
    RCPtr<SkinnedMesh> SkinnedMesh::StaticCreate(
        string_view                      name,
        RCPtr<Skeleton>                  skeleton,
        array_list<SkinnedMeshSection>&& sections,
        array_list<string>&&             materialNames)
    {
        auto self = NewAssetObject<SkinnedMesh>();
        self->SetIndexName(name);
        self->m_skeleton      = skeleton;
        self->m_sections      = std::move(sections);
        self->m_materialNames = std::move(materialNames);
        return self;
    }

    void SkinnedMesh::OnCollectAssetDependencies(array_list<guid_t>& deps)
    {
        base::OnCollectAssetDependencies(deps);
        if (m_skeleton)
            deps.push_back(m_skeleton.GetGuid());
    }

    void SkinnedMesh::OnInstantiateAsset(AssetObject* obj)
    {
        base::OnInstantiateAsset(obj);
        auto mesh = static_cast<ThisClass*>(obj);
        mesh->m_skeleton  = m_skeleton;
        mesh->m_sections  = m_sections;
    }

    SkinnedMesh::~SkinnedMesh() = default;

} // namespace pulsar
