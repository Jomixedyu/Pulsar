#include "Assets/SkinnedMesh.h"

#include "Application.h"
#include "AssetSerializerUtil.h"
#include "EngineMath.h"

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
            v.Normal   = Normals.empty()  ? Vector3f{0, 1, 0}      : Normals[i];
            v.Tangent  = Tangents.empty() ? Vector4f{1, 0, 0, 1}   : Tangents[i];
            v.Color    = Colors.empty()   ? Color4b{255, 255, 255, 255} : Colors[i];

            for (uint8_t uvIdx = 0; uvIdx < NumTexCoords && uvIdx < STATICMESH_MAX_TEXTURE_COORDS; uvIdx++)
            {
                v.TexCoords[uvIdx] = TexCoords[uvIdx][i];
            }

            if (!BoneIndices.empty())
            {
                v.BoneIndices[0] = BoneIndices[i][0];
                v.BoneIndices[1] = BoneIndices[i][1];
                v.BoneIndices[2] = BoneIndices[i][2];
                v.BoneIndices[3] = BoneIndices[i][3];
                v.BoneWeights[0] = BoneWeights[i][0];
                v.BoneWeights[1] = BoneWeights[i][1];
                v.BoneWeights[2] = BoneWeights[i][2];
                v.BoneWeights[3] = BoneWeights[i][3];
            }
        }
        return result;
    }

    // -----------------------------------------------------------------------
    // Serialization helpers
    // -----------------------------------------------------------------------
    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, BoneInfo& data)
    {
        sser::ReadWriteStream(stream, isWrite, data.Name);
        sser::ReadWriteStream(stream, isWrite, data.ParentIndex);
        sser::ReadWriteStream(stream, isWrite, data.InverseBindMatrix);
        return stream;
    }

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
        sser::ReadWriteStream(stream, isWrite, data.BoneIndices);
        sser::ReadWriteStream(stream, isWrite, data.BoneWeights);
        return stream;
    }

    // -----------------------------------------------------------------------
    // Vertex layout（SkinnedMesh 专用，额外带 BoneIndices / BoneWeights）
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

        for (size_t i = 0; i < STATICMESH_MAX_TEXTURE_COORDS; i++)
        {
            layout->Attributes.push_back({(int)EngineInputSemantic::TEXCOORD0 + i, gfx::GFXVertexInputDataFormat::R32G32_SFloat, offsetof(SkinnedMeshVertex, TexCoords[i])});
        }

        // BoneIndices: uint4（用 R32G32B32A32_UInt 语义）
        layout->Attributes.push_back({(int)EngineInputSemantic::BLENDINDICES, gfx::GFXVertexInputDataFormat::R32G32B32A32_UInt,  offsetof(SkinnedMeshVertex, BoneIndices)});
        // BoneWeights: float4
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
        if (m_isCreatedResource)
            return true;
        m_isCreatedResource = true;

        for (auto& section : m_sections)
        {
            auto interleavedVerts = section.BuildInterleavedVertices();
            const size_t vertSize = interleavedVerts.size() * sizeof(SkinnedMeshVertex);

            {
                gfx::GFXBufferDesc vertexDesc{};
                vertexDesc.Usage       = gfx::GFXBufferUsage::Vertex;
                vertexDesc.StorageType = gfx::GFXBufferMemoryPosition::DeviceLocal;
                vertexDesc.BufferSize  = vertSize;
                vertexDesc.ElementSize = sizeof(SkinnedMeshVertex);

                auto vertBuffer = Application::GetGfxApp()->CreateBuffer(vertexDesc);
                vertBuffer->Fill(interleavedVerts.data());
                m_vertexBuffers.push_back(vertBuffer);
            }

            {
                gfx::GFXBufferDesc indicesDesc{};
                indicesDesc.Usage       = gfx::GFXBufferUsage::Indices;
                indicesDesc.StorageType = gfx::GFXBufferMemoryPosition::DeviceLocal;
                indicesDesc.BufferSize  = section.GetIndicesAllocSize();
                indicesDesc.ElementSize = sizeof(MeshIndicesType);

                auto indicesBuffer = Application::GetGfxApp()->CreateBuffer(indicesDesc);
                indicesBuffer->Fill(section.Indices.data());
                m_indicesBuffers.push_back(indicesBuffer);
            }
        }
        return true;
    }

    void SkinnedMesh::DestroyGPUResource()
    {
        if (!m_isCreatedResource)
            return;
        m_isCreatedResource = false;
        m_vertexBuffers.clear();
        m_indicesBuffers.clear();
    }

    bool SkinnedMesh::IsCreatedGPUResource() const
    {
        return m_isCreatedResource;
    }

    // -----------------------------------------------------------------------
    // Serialize
    // -----------------------------------------------------------------------
    void SkinnedMesh::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        if (!s->IsWrite)
        {
            m_bones.clear();
            m_sections.clear();
            m_materialNames.clear();
        }

        sser::ReadWriteStream(s->GetStream(), s->IsWrite, m_bones);
        sser::ReadWriteStream(s->GetStream(), s->IsWrite, m_sections);

        if (s->IsWrite)
        {
            auto materialNames = s->Object->New(ser::VarientType::Array);
            for (auto& name : m_materialNames)
                materialNames->Push(name);
            s->Object->Add("MaterialNames", materialNames);
        }
        else
        {
            if (auto materialNames = s->Object->At("MaterialNames"))
            {
                for (int i = 0; i < materialNames->GetCount(); ++i)
                    m_materialNames.push_back(materialNames->At(i)->AsString());
            }
        }
    }

    // -----------------------------------------------------------------------
    // StaticCreate
    // -----------------------------------------------------------------------
    RCPtr<SkinnedMesh> SkinnedMesh::StaticCreate(
        string_view                      name,
        array_list<BoneInfo>&&           bones,
        array_list<SkinnedMeshSection>&& sections,
        array_list<string>&&             materialNames)
    {
        auto self = NewAssetObject<SkinnedMesh>();
        self->SetIndexName(name);
        self->m_bones         = std::move(bones);
        self->m_sections      = std::move(sections);
        self->m_materialNames = std::move(materialNames);
        return self;
    }

    int32_t SkinnedMesh::FindBoneIndex(const string& name) const
    {
        for (int32_t i = 0; i < (int32_t)m_bones.size(); ++i)
        {
            if (m_bones[i].Name == name)
                return i;
        }
        return -1;
    }

    void SkinnedMesh::OnInstantiateAsset(AssetObject* obj)
    {
        base::OnInstantiateAsset(obj);
        auto mesh = static_cast<ThisClass*>(obj);
        mesh->m_bones    = m_bones;
        mesh->m_sections = m_sections;
    }

    SkinnedMesh::~SkinnedMesh() = default;

} // namespace pulsar
