#include "Assets/StaticMesh.h"

#include "Application.h"
#include "AssetSerializerUtil.h"

#include "EngineMath.h"
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Assets/Texture2D.h>

namespace pulsar
{
    static auto _GetVertexLayout()
    {
        auto vertDescLayout = Application::GetGfxApp()->CreateVertexLayoutDescription();
        vertDescLayout->BindingPoint = 0;
        vertDescLayout->Stride = sizeof(StaticMeshVertex);

        vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::POSITION,  gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Position)});
        vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::NORMAL,    gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Normal)});
        vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::TANGENT,   gfx::GFXVertexInputDataFormat::R32G32B32A32_SFloat, offsetof(StaticMeshVertex, Tangent)});
        vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::COLOR,     gfx::GFXVertexInputDataFormat::R8G8B8A8_UNorm,   offsetof(StaticMeshVertex, Color)});

        for (size_t i = 0; i < STATICMESH_MAX_TEXTURE_COORDS; i++)
        {
            vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::TEXCOORD0 + i, gfx::GFXVertexInputDataFormat::R32G32_SFloat, offsetof(StaticMeshVertex, TexCoords[i])});
        }

        return vertDescLayout;
    }

    // 将按属性分离的序列化数据合并为交错格式用于 GPU 上传
    array_list<StaticMeshVertex> StaticMeshSection::BuildInterleavedVertices() const
    {
        const size_t vertCount = Positions.size();
        array_list<StaticMeshVertex> result(vertCount);

        for (size_t i = 0; i < vertCount; i++)
        {
            StaticMeshVertex& v = result[i];
            v.Position = Positions[i];
            v.Normal   = Normals.empty()   ? Vector3f{0, 1, 0} : Normals[i];
            v.Tangent  = Tangents.empty()  ? Vector4f{1, 0, 0, 1} : Tangents[i];
            v.Color    = Colors.empty()    ? Color4b{255, 255, 255, 255} : Colors[i];

            for (uint8_t uvIdx = 0; uvIdx < NumTexCoords && uvIdx < STATICMESH_MAX_TEXTURE_COORDS; uvIdx++)
            {
                v.TexCoords[uvIdx] = TexCoords[uvIdx][i];
            }
        }

        return result;
    }

    void StaticMesh::OnInstantiateAsset(AssetObject* obj)
    {
        base::OnInstantiateAsset(obj);
        auto mesh = static_cast<ThisClass*>(obj);
        mesh->m_sections = m_sections;
    }

    bool StaticMesh::CreateGPUResource()
    {
        if (m_isCreatedResource)
        {
            return true;
        }
        m_isCreatedResource = true;
        for (auto& section : m_sections)
        {
            // 从分离属性数据合并为交错格式再上传（GPU 侧暂时保持单 Buffer）
            auto interleavedVerts = section.BuildInterleavedVertices();
            const size_t vertSize = interleavedVerts.size() * sizeof(StaticMeshVertex);

            {
                gfx::GFXBufferDesc vertexDesc{};
                vertexDesc.Usage       = gfx::GFXBufferUsage::Vertex;
                vertexDesc.StorageType = gfx::GFXBufferMemoryPosition::DeviceLocal;
                vertexDesc.BufferSize  = vertSize;
                vertexDesc.ElementSize = sizeof(StaticMeshVertex);

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
    void StaticMesh::DestroyGPUResource()
    {
        if (!m_isCreatedResource)
            return;
        m_isCreatedResource = false;

        m_vertexBuffers.clear();
        m_indicesBuffers.clear();
    }
    bool StaticMesh::IsCreatedGPUResource() const
    {
        return m_isCreatedResource;
    }

    StaticMesh::~StaticMesh() = default;

    gfx::GFXVertexLayoutDescription_sp StaticMesh::StaticGetVertexLayout()
    {
        static gfx::GFXVertexLayoutDescription_wp layout;
        if (layout.expired())
        {
            auto newLayout = _GetVertexLayout();
            layout = newLayout;
            return newLayout;
        }
        return layout.lock();
    }

    void StaticMesh::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        if (!s->IsWrite)
        {
            m_sections.clear();
            m_materialNames.clear();
        }

        sser::ReadWriteStream(s->GetStream(), s->IsWrite, m_sections);
        if (s->IsWrite)
        {
            auto materialNames = s->Object->New(ser::VarientType::Array);
            for (auto& name : m_materialNames)
            {
                materialNames->Push(name);
            }
            s->Object->Add("MaterialNames", materialNames);

            s->Object->Add("Bounds", AssetSerializerUtil::NewObject(s->Object, m_bounds));
        }
        else
        {
            if (auto materialNames = s->Object->At("MaterialNames"))
            {
                for (int i = 0; i < materialNames->GetCount(); ++i)
                {
                    m_materialNames.push_back(materialNames->At(i)->AsString());
                }
            }

            if (auto bound = s->Object->At("Bounds"))
            {
                m_bounds = AssetSerializerUtil::GetBounds3Object(bound);
            }
        }
    }

    RCPtr<StaticMesh> StaticMesh::StaticCreate(
        string_view name,
        array_list<StaticMeshSection>&& vertData,
        array_list<string>&& materialNames)
    {
        auto self = NewAssetObject<StaticMesh>();
        self->SetIndexName(name);
        self->m_sections = std::move(vertData);
        self->m_materialNames = std::move(materialNames);

        self->CalcBounds();

        return self;
    }
    void StaticMesh::CalcBounds()
    {
        array_list<Vector3f> verties;
        size_t pointArrSize = 0;

        for (auto& section : m_sections)
        {
            pointArrSize += section.Indices.size();
        }
        verties.reserve(pointArrSize);

        for (auto& section : m_sections)
        {
            for (uint32_t index : section.Indices)
            {
                verties.push_back(section.Positions[index]);
            }
        }

        m_bounds = BoxSphereBounds3f::CreateFromPoints(verties.data(), verties.size());
    }

    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, StaticMeshSection& data)
    {
        sser::ReadWriteStream(stream, isWrite, data.NumTexCoords);
        sser::ReadWriteStream(stream, isWrite, data.Positions);
        sser::ReadWriteStream(stream, isWrite, data.Normals);
        sser::ReadWriteStream(stream, isWrite, data.Tangents);
        sser::ReadWriteStream(stream, isWrite, data.Colors);
        sser::ReadWriteStream(stream, isWrite, data.TexCoords);
        sser::ReadWriteStream(stream, isWrite, data.Indices);
        sser::ReadWriteStream(stream, isWrite, data.MaterialIndex);
        return stream;
    }

} // namespace pulsar