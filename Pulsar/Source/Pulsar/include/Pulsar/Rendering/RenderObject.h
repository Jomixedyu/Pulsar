#pragma once
#include <gfx/GFXApplication.h>
#include <gfx/GFXBuffer.h>
#include <Pulsar/EngineMath.h>
#include <Pulsar/Assets/Material.h>

namespace pulsar::rendering
{

    struct MeshBatchElement
    {
        gfx::GFXBuffer_sp Vertex;
        gfx::GFXBuffer_sp Indices;
        Matrix4f LocalToWorldMatrix;
    };

    struct MeshBatch
    {
        array_list<MeshBatchElement> Elements;

        Material_ref Material;

        bool IsUsedIndices;
        bool IsWireframe;
        bool IsCastShadow;
        bool IsReverseCulling;
        gfx::GFXPrimitiveTopology Topology;

        bool IsSameRenderingState(const MeshBatch& batch) const
        {
            return
                Material == batch.Material &&
                IsReverseCulling == batch.IsReverseCulling;
        }

        void Append(const MeshBatch& batch)
        {
            for (const auto& element : batch.Elements)
            {
                Elements.push_back(element);
            }
        }
        void Append(MeshBatch&& batch)
        {
            for (auto& element : batch.Elements)
            {
                Elements.push_back(std::move(element));
            }
        };
    };

    class IDrawInterface
    {
    public:

    };

    class RenderObject
    {
    public:
        virtual ~RenderObject() = default;
        void SetTransform(const Matrix4f& localToWorld)
        {
            m_localToWorld = localToWorld;
            m_isLocalToWorldDeterminantNegative = localToWorld.Determinant() < 0;

            UpdateConstantBuffer();
        }
        void UpdateConstantBuffer()
        {

        }

        virtual void OnCreateResource() {}
        virtual void OnDestroyResource() {}

        virtual array_list<MeshBatch> GetMeshBatchs() = 0;
        virtual bool IsActive() const { return m_active; };

        bool IsDetermiantNegative() const { return m_isLocalToWorldDeterminantNegative; }

    public:

    protected:
        bool      m_active = false;
        Matrix4f  m_localToWorld{1};
        bool      m_isLocalToWorldDeterminantNegative{};

    };
    CORELIB_DECL_SHORTSPTR(RenderObject);
}