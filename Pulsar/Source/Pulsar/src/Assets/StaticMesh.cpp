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

        vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::POSITION, gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Position)});
        vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::NORMAL, gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Normal)});
        vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::TANGENT, gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Tangent)});
        vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::BITANGENT, gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Bitangent)});
        vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::COLOR, gfx::GFXVertexInputDataFormat::R32G32B32_SFloat, offsetof(StaticMeshVertex, Color)});

        for (size_t i = 0; i < STATICMESH_MAX_TEXTURE_COORDS; i++)
        {
            vertDescLayout->Attributes.push_back({(int)EngineInputSemantic::TEXCOORD0 + i, gfx::GFXVertexInputDataFormat::R32G32_SFloat, offsetof(StaticMeshVertex, TexCoords[i])});
        }

        return vertDescLayout;
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
            auto vertSize = section.Vertex.size() * kSizeofStaticMeshVertex;
            auto vertBuffer = Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::Vertex, vertSize);
            vertBuffer->Fill(section.Vertex.data());
            vertBuffer->SetElementCount(section.Vertex.size());
            m_vertexBuffers.push_back(vertBuffer);

            auto indicesSize = section.Indices.size() * sizeof(decltype(section.Indices)::value_type);
            auto indicesBuffer = Application::GetGfxApp()->CreateBuffer(gfx::GFXBufferUsage::Index, indicesSize);
            indicesBuffer->Fill(section.Indices.data());
            indicesBuffer->SetElementCount(section.Indices.size());
            m_indicesBuffers.push_back(indicesBuffer);
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

        sser::ReadWriteStream(s->Stream, s->IsWrite, m_sections);
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
        StaticMesh_sp self = mksptr(new StaticMesh);
        self->Construct();
        self->SetIndexName(name);
        self->m_sections = std::move(vertData);
        self->m_materialNames = std::move(materialNames);

        self->CalcBounds();

        return self;
    }
    void StaticMesh::CalcBounds()
    {
        Box3f box;

        bool init = false;
        for (auto& section : m_sections)
        {
            for (uint32_t index : section.Indices)
            {
                // min max
                auto& pos = section.Vertex[index].Position;
                if (!init)
                {
                    box.Min = pos;
                    box.Max = pos;
                    init = true;
                    continue;
                }
                if (pos.x < box.Min.x)
                    box.Min.x = pos.x;
                if (pos.y < box.Min.y)
                    box.Min.y = pos.y;
                if (pos.z < box.Min.z)
                    box.Min.z = pos.z;
                if (pos.x > box.Max.x)
                    box.Max.x = pos.x;
                if (pos.y > box.Max.y)
                    box.Max.y = pos.y;
                if (pos.z > box.Max.z)
                    box.Max.z = pos.z;
            }
        }

        constexpr auto minFloat = 0.00001f;
        if (box.Min.x == box.Max.x)
            box.Max.x = minFloat;;
        if (box.Min.y == box.Max.y)
            box.Max.y = minFloat;
        if (box.Min.z == box.Max.z)
            box.Max.z = minFloat;

        m_bounds = Bounds3f{box};
    }

    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, StaticMeshVertex& data)
    {
        ReadWriteStream(stream, isWrite, data.Position);
        ReadWriteStream(stream, isWrite, data.Normal);
        ReadWriteStream(stream, isWrite, data.Tangent);
        ReadWriteStream(stream, isWrite, data.Bitangent);
        ReadWriteStream(stream, isWrite, data.Color);
        sser::ReadWriteStream(stream, isWrite, data.TexCoords);
        return stream;
    }

    std::iostream& ReadWriteStream(std::iostream& stream, bool isWrite, StaticMeshSection& data)
    {
        sser::ReadWriteStream(stream, isWrite, data.Vertex);
        sser::ReadWriteStream(stream, isWrite, data.Indices);
        sser::ReadWriteStream(stream, isWrite, data.MaterialIndex);
        return stream;
    }

} // namespace pulsar