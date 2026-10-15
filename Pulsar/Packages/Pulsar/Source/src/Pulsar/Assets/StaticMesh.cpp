#include "Assets/StaticMesh.h"

#include <Pulsar/Rendering/MeshProxy.h>
#include <Pulsar/Application.h>
#include <Pulsar/Rendering/RenderThread.h>

#include "AssetSerializerUtil.h"

#include "EngineMath.h"
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Assets/Texture2D.h>

namespace pulsar
{
    static gfx::GFXVertexLayoutDescription _GetVertexLayout()
    {
        gfx::GFXVertexLayoutDescription vertDescLayout;
        vertDescLayout.BindingPoint = 0;
        vertDescLayout.Stride = sizeof(StaticMeshVertex);

        vertDescLayout.Attributes.push_back({(int)EngineInputSemantic::POSITION,  offsetof(StaticMeshVertex, Position), gfx::GFXVertexInputDataFormat::R32G32B32_SFloat});
        vertDescLayout.Attributes.push_back({(int)EngineInputSemantic::NORMAL,    offsetof(StaticMeshVertex, Normal), gfx::GFXVertexInputDataFormat::R32G32B32_SFloat});
        vertDescLayout.Attributes.push_back({(int)EngineInputSemantic::TANGENT,   offsetof(StaticMeshVertex, Tangent), gfx::GFXVertexInputDataFormat::R32G32B32A32_SFloat});
        vertDescLayout.Attributes.push_back({(int)EngineInputSemantic::COLOR,     offsetof(StaticMeshVertex, Color), gfx::GFXVertexInputDataFormat::R8G8B8A8_UNorm});

        for (size_t i = 0; i < STATICMESH_MAX_TEXTURE_COORDS; i++)
        {
            vertDescLayout.Attributes.push_back({(uint32_t)((int)EngineInputSemantic::TEXCOORD0 + i), (uint32_t)offsetof(StaticMeshVertex, TexCoords[i]), gfx::GFXVertexInputDataFormat::R32G32_SFloat});
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

    bool StaticMesh::EnsureGPUResource()
    {
        if (m_isCreatedResource)
        {
            return true;
        }
        m_isCreatedResource = true;

        m_proxy = std::make_shared<rendering::MeshProxy>();
        auto proxy = m_proxy;
        auto sections = rendering::BuildMeshProxySections<StaticMeshSection, StaticMeshVertex>(m_sections);
        Application::GetRenderThread()->EnqueueUpdate_AnyThread(
            [proxy = std::move(proxy), sections = std::move(sections)](gfx::GFXResourceRegistry*) mutable
            {
                proxy->Init(std::move(sections));
            });
        return true;
    }

    void StaticMesh::DestroyGPUResource()
    {
        if (!m_isCreatedResource)
            return;
        m_isCreatedResource = false;
        if (auto proxy = std::move(m_proxy))
        {
            Application::GetRenderThread()->EnqueueDestroy_AnyThread(
                [proxy = std::move(proxy)](gfx::GFXResourceRegistry*) mutable
                {
                    proxy->Release();
                });
        }
    }    bool StaticMesh::IsCreatedGPUResource() const
    {
        return m_isCreatedResource;
    }

    StaticMesh::~StaticMesh() = default;

    void StaticMesh::OnDestroy()
    {
        base::OnDestroy();
        DestroyGPUResource();
    }

    gfx::GFXVertexLayoutDescription StaticMesh::StaticGetVertexLayout()
    {
        static gfx::GFXVertexLayoutDescription layout = _GetVertexLayout();
        return layout;
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
