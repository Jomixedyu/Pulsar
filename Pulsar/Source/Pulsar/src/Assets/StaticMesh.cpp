#include "Assets/StaticMesh.h"
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Assets/Texture2D.h>
#include "EngineMath.h"

namespace pulsar
{
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool isWrite, StaticMeshSectionSerializeData& data)
    {
        using namespace pulsar::math;
        using namespace jxcorlib::math;
        using namespace ser;
        using namespace jmath;
        ReadWriteStream(stream, isWrite, data.Position);
        ser::ReadWriteStream(stream, isWrite, data.Normal);
        ser::ReadWriteStream(stream, isWrite, data.Bitangent);
        ser::ReadWriteStream(stream, isWrite, data.Indices);
        ser::ReadWriteStream(stream, isWrite, data.Color);

        int32_t coordCount = isWrite ? data.TexCoordsCount : 0;
        ser::ReadWriteStream(stream, isWrite, coordCount);

        for (int32_t i = 0; i < coordCount; i++)
        {
            ser::ReadWriteStream(stream, isWrite, data.TexCoords[i]);
        }
        return stream;
    }
    ser::Stream& ReadWriteStream(ser::Stream& stream, bool isWrite, StaticMeshSerializeData& data)
    {
        using namespace math;
        using namespace ser;

        ReadWriteStream(stream, isWrite, data.Sections);
        ReadWriteStream(stream, isWrite, data.Materials);

        return stream;
    }

    void StaticMesh::OnInstantiateAsset(sptr<AssetObject>& obj)
    {
        auto mesh = sptr_cast<StaticMesh>(obj);
        assert(mesh);
        mesh->m_sections = m_sections;
        mesh->m_materials = m_materials;
        base::OnInstantiateAsset(obj);
    }

    StaticMesh::~StaticMesh()
    {

    }

    StaticMesh_sp StaticMesh::StaticCreate(StaticMeshSerializeData&& vertData)
    {
        auto mesh = mksptr(new StaticMesh);
        mesh->Construct();
        mesh->m_sections = std::move(vertData.Sections);
        mesh->m_materials.reserve(vertData.Materials.size());

        for (auto& mat : vertData.Materials)
        {
            mesh->m_materials.push_back(mat.Get());
        }

        return mesh;
    }




}