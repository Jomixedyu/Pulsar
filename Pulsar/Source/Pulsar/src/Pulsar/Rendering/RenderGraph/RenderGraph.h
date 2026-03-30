#pragma once
#include "TransientRTPool.h"
#include <Pulsar/Assets/RenderTexture.h>
#include <gfx/GFXRenderPass.h>
#include <functional>
#include <string>
#include <vector>
#include <unordered_map>

namespace gfx { class GFXCommandBuffer; }

namespace pulsar
{
    class PerPassResources;

    struct RGTextureHandle
    {
        static constexpr uint32_t Invalid = UINT32_MAX;
        uint32_t id = Invalid;
        bool IsValid() const { return id != Invalid; }
        bool operator==(const RGTextureHandle& o) const { return id == o.id; }
        bool operator!=(const RGTextureHandle& o) const { return id != o.id; }
    };

    class RenderGraph;

    class RGPassContext
    {
    public:
        RGPassContext(const std::vector<RCPtr<RenderTexture>>& physicalRTs,
                      const std::unordered_map<uint32_t, size_t>& handleToIndex,
                      PerPassResources* perPassResources)
            : m_physicalRTs(physicalRTs)
            , m_handleToIndex(handleToIndex)
            , perPassResources(perPassResources) {}

        RenderTexture* Get(RGTextureHandle h) const
        {
            auto it = m_handleToIndex.find(h.id);
            if (it == m_handleToIndex.end()) return nullptr;
            return m_physicalRTs[it->second].GetPtr();
        }

        PerPassResources* perPassResources = nullptr;

    private:
        const std::vector<RCPtr<RenderTexture>>& m_physicalRTs;
        const std::unordered_map<uint32_t, size_t>& m_handleToIndex;
    };

    using RGPassExecuteFunc = std::function<void(RGPassContext&, gfx::GFXCommandBuffer&)>;

    enum class RGResourceUsage : uint8_t
    {
        Read,
        Write,
    };

    using RGAttachmentDesc = gfx::GFXRenderPassBeginInfo;

    struct RGResourceBinding
    {
        RGTextureHandle  handle;
        RGResourceUsage  usage;
        RGAttachmentDesc attachment; // only meaningful for Write
    };

    struct RGPassDesc
    {
        std::string name;
        std::vector<RGResourceBinding>  resources;
        RGPassExecuteFunc               executeFunc;
        PerPassResources*               perPassResources = nullptr;
    };

    enum class RGResourceKind : uint8_t { Transient, Persistent };

    struct RGResourceDesc
    {
        std::string     name;
        RGResourceKind  kind     = RGResourceKind::Transient;
        RGTextureDesc   texDesc;
        RenderTexture*  external = nullptr;
    };

    class RGPassBuilder
    {
    public:
        explicit RGPassBuilder(RGPassDesc& pass) : m_pass(pass) {}

        RGPassBuilder& Read(RGTextureHandle h, RGAttachmentDesc desc = {})
        {
            m_pass.resources.push_back({h, RGResourceUsage::Read, desc});
            return *this;
        }

        RGPassBuilder& Write(RGTextureHandle h, RGAttachmentDesc desc = {})
        {
            m_pass.resources.push_back({h, RGResourceUsage::Write, desc});
            return *this;
        }

        RGPassBuilder& WithPerPass(PerPassResources* perPass)
        {
            m_pass.perPassResources = perPass;
            return *this;
        }

        RGPassBuilder& Execute(RGPassExecuteFunc func)
        {
            m_pass.executeFunc = std::move(func);
            return *this;
        }

    private:
        RGPassDesc& m_pass;
    };

    class RenderGraph
    {
    public:
        RenderGraph() = default;
        ~RenderGraph() { Reset(); }

        void BeginFrame();
        RGTextureHandle CreateTransient(const std::string& name, const RGTextureDesc& desc);
        RGTextureHandle ImportTexture(const std::string& name, RenderTexture* external);
        RGPassBuilder AddPass(const std::string& name);
        bool Compile();
        void Execute(gfx::GFXCommandBuffer& cmd);
        void Reset();

    private:
        std::vector<RGResourceDesc>                  m_resources;
        std::vector<RGPassDesc>                      m_passes;
        std::vector<size_t>                          m_sortedPassIndices;
        std::vector<RCPtr<RenderTexture>>            m_physicalRTs;
        std::unordered_map<uint32_t, size_t>         m_handleToRTIndex;

        struct AcquiredEntry { RGTextureDesc desc; size_t rtIndex; };
        std::vector<AcquiredEntry>                   m_acquiredTransients;

        bool m_compiled = false;

        uint32_t NextHandleId() { return static_cast<uint32_t>(m_resources.size()); }
    };
}
