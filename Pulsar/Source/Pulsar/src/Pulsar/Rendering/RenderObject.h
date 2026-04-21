#pragma once
#include "PrimitiveStruct.h"
#include "ShaderConfig.h"

#include <Pulsar/Assets/Material.h>
#include <Pulsar/EngineMath.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXBuffer.h>

namespace pulsar::rendering
{

    struct MeshBatchElement
    {
        gfx::GFXBuffer_sp Vertex;
        gfx::GFXBuffer_sp Indices;
        gfx::GFXDescriptorSet_sp ModelDescriptor;
    };

    struct MeshBatch
    {
        array_list<MeshBatchElement> Elements;
        gfx::GFXDescriptorSetLayout_sp DescriptorSetLayout;
        RCPtr<Material> Material;
        std::string Interface; // Renderer interface name (e.g. "RENDERER_STATICMESH")

        gfx::GFXGraphicsPipelineState State{};
        bool IsUsedIndices{};
        bool IsWireframe{};
        bool IsCastShadow{};
        bool IsDepthTestDisabled{false};  // Gizmo 等需要始终可见的线条
        gfx::GFXCullMode CullMode{};
        bool IsReverseCulling{false};

        // Sorting metadata (filled by RenderObject or pipeline)
        ShaderPassRenderQueueType Queue = ShaderPassRenderQueueType::Opaque;
        int32_t Priority = 0;   // per-material render priority
        float   Depth    = 0.f; // signed camera-space depth, filled by pipeline

        size_t GetRenderState() const
        {
            constexpr size_t prime = 16777619;
            return (2166136261 * prime
                ^ std::hash<RCPtrBase>()(Material)) * prime
                ^ State.GetHashCode() * prime;
        }

        gfx::GFXCullMode GetCullMode() const
        {
            if (IsReverseCulling)
            {
                if (CullMode == gfx::GFXCullMode::Front)
                {
                    return gfx::GFXCullMode::Back;
                }
                if (CullMode == gfx::GFXCullMode::Back)
                {
                    return gfx::GFXCullMode::Front;
                }
            }
            return CullMode;
        }

        void Append(const MeshBatch& batch)
        {
            for (const auto& element : batch.Elements)
            {
                Elements.push_back(element);
            }
        }
    };


    class RenderObject
    {
    public:
        virtual ~RenderObject() = default;
        void SetTransform(const Matrix4f& localToWorld);
        virtual void OnChangedTransform() {}
        void UpdateConstantBuffer()
        {

        }

        virtual void OnCreateResource() {}
        virtual void OnDestroyResource() {}

        virtual array_list<MeshBatch> GetMeshBatches() = 0;
        virtual std::string GetInterface() const { return {}; }
        virtual bool IsActive() const { return m_active; };

        bool IsDeterminantNegative() const { return m_isLocalToWorldDeterminantNegative; }

        // Returns world-space position (column 3 of LocalToWorld matrix)
        Vector3f GetWorldPosition() const
        {
            const auto& col3 = m_perModelData.LocalToWorldMatrix.GetColumn(3);
            return Vector3f{col3.x, col3.y, col3.z};
        }

    public:

    protected:
        bool      m_active = false;
        PerRendererData  m_perModelData{};
        bool      m_isLocalToWorldDeterminantNegative{};
        int       m_lineWidth{1};
    };
    CORELIB_DECL_SHORTSPTR(RenderObject);
}