#pragma once
#include "PrimitiveStruct.h"
#include "ShaderConfig.h"

#include <Pulsar/Assets/Material.h>
#include <Pulsar/EngineMath.h>
#include <gfx/GFXApplication.h>
#include <gfx/GFXBuffer.h>
#include <gfx/GFXHandle.h>

namespace pulsar
{
    class PerRenderObjectDataManager;
}

namespace pulsar::rendering
{

    struct MeshBatchElement
    {
        gfx::BufferHandle Vertex;
        gfx::BufferHandle Indices;
    };

    struct MeshBatch
    {
        array_list<MeshBatchElement> Elements;
        gfx::GFXDescriptorSetLayout_sp DescriptorSetLayout;
        gfx::GFXDescriptorSet_sp ExtraDescriptorSet; // set2 (dummy or skinning)
        RCPtr<Material> Material;
        std::string Interface; // Renderer interface name (e.g. "RENDERER_STATICMESH")

        gfx::GFXGraphicsPipelineState State{};
        bool IsUsedIndices{};
        bool IsWireframe{};
        bool IsCastShadow{};
        bool IsDepthTestDisabled{false};  // Gizmo 等需要始终可见的线条
        bool IsReverseCulling{false};

        int32_t Priority = 0;   // per-renderer render priority
        float   Depth    = 0.f; // signed camera-space depth, filled by pipeline
        uint32_t RenderObjectIndex = 0; // dynamic offset index for set1 binding3

        size_t GetRenderState() const
        {
            constexpr size_t prime = 16777619;
            return (2166136261 * prime
                ^ std::hash<RCPtrBase>()(Material)) * prime
                ^ State.GetHashCode() * prime;
        }

        gfx::GFXCullMode GetCullMode(gfx::GFXCullMode baseCullMode) const
        {
            if (IsReverseCulling)
            {
                if (baseCullMode == gfx::GFXCullMode::Front)
                    return gfx::GFXCullMode::Back;
                if (baseCullMode == gfx::GFXCullMode::Back)
                    return gfx::GFXCullMode::Front;
            }
            return baseCullMode;
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
        static constexpr uint32_t kInvalidSlot = UINT32_MAX;

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
            const auto& col3 = m_perRenderObjectData.LocalToWorldMatrix.GetColumn(3);
            return Vector3f{col3.x, col3.y, col3.z};
        }

        void SetRenderObjectIndex(uint32_t index) { m_renderObjectIndex = index; }
        uint32_t GetRenderObjectIndex() const { return m_renderObjectIndex; }

        void SetPerRenderObjectDataManager(PerRenderObjectDataManager* mgr) { m_pPerRenderObjectDataManager = mgr; }
        PerRenderObjectDataManager* GetPerRenderObjectDataManager() const { return m_pPerRenderObjectDataManager; }

        const PerRenderObjectData& GetPerRenderObjectData() const { return m_perRenderObjectData; }
        void SetPerRenderObjectData(const PerRenderObjectData& data) { m_perRenderObjectData = data; }

    public:

    protected:
        bool      m_active = false;
        PerRenderObjectData  m_perRenderObjectData{};
        bool      m_isLocalToWorldDeterminantNegative{};
        int       m_lineWidth{1};
        uint32_t  m_renderObjectIndex = kInvalidSlot;
        PerRenderObjectDataManager* m_pPerRenderObjectDataManager = nullptr;
    };
    CORELIB_DECL_SHORTSPTR(RenderObject);
}
