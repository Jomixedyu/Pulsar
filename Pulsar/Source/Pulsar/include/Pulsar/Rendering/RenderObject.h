#pragma once
#include <gfx/GFXApplication.h>
#include <gfx/GFXBuffer.h>
#include "EngineMath.h"
#include "Assets/Material.h"

namespace pulsar::rendering
{

    struct MeshBatch
    {
        array_list<gfx::GFXBuffer_sp> Vertex;
        array_list<gfx::GFXBuffer_sp> Indices;
        Material_sp Material;

        void Append(const MeshBatch& batch)
        {
            if (Material != batch.Material)
            {
                return;
            }
            Vertex.insert(Vertex.end(), batch.Vertex.begin(), batch.Vertex.end());
            Indices.insert(Indices.end(), batch.Indices.begin(), batch.Indices.end());

        };
    };

    class IDrawInterface
    {
    public:

    };

    class RenderObject
    {
    public:
        void SetTransform(const Matrix4f& localToWorld)
        {
            m_localToWorld = localToWorld;
            //m_isLocalToWorldDeterminantNegative = localToWorld.Determinant() < 0;
            m_isLocalToWorldDeterminantNegative = false;
            UpdateConstantBuffer();
        }
        void UpdateConstantBuffer()
        {

        }

        array_list<MeshBatch> GetMeshBatchs();
    protected:
        Matrix4f  m_localToWorld;
        bool      m_isLocalToWorldDeterminantNegative;

    };
}