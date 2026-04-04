#include "RenderGraph.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Assets/RenderTexture.h>
#include <gfx/GFXCommandBuffer.h>
#include <gfx/GFXTexture.h>
#include <algorithm>
#include <cassert>

namespace pulsar
{
    void RenderGraph::BeginFrame()
    {
        Reset();

        m_resources.clear();
        m_passes.clear();
        m_sortedPassIndices.clear();
        m_physicalRTs.clear();
        m_handleToRTIndex.clear();
        m_compiled = false;
    }

    RGTextureHandle RenderGraph::CreateTransient(const std::string& name, const RGTextureDesc& desc)
    {
        RGTextureHandle h{ NextHandleId() };
        RGResourceDesc res;
        res.name    = name;
        res.kind    = RGResourceKind::Transient;
        res.texDesc = desc;
        m_resources.push_back(std::move(res));
        return h;
    }

    RGTextureHandle RenderGraph::ImportTexture(const std::string& name, RenderTexture* external)
    {
        assert(external && "ImportTexture: external RT must not be null");
        RGTextureHandle h{ NextHandleId() };
        RGResourceDesc res;
        res.name     = name;
        res.kind     = RGResourceKind::Persistent;
        res.external = external;
        m_resources.push_back(std::move(res));
        return h;
    }

    RGPassBuilder RenderGraph::AddPass(const std::string& name)
    {
        m_passes.push_back(RGPassDesc{ name, {}, nullptr });
        return RGPassBuilder(m_passes.back());
    }

    bool RenderGraph::Compile()
    {
        m_sortedPassIndices.clear();
        m_physicalRTs.clear();
        m_handleToRTIndex.clear();
        m_acquiredTransients.clear();

        const size_t passCount = m_passes.size();
        const size_t resCount  = m_resources.size();

        m_physicalRTs.resize(resCount, nullptr);

        std::vector<int> inDegree(passCount, 0);
        std::vector<std::vector<size_t>> passEdges(passCount);
        std::vector<int> latestWriterForResource(resCount, -1);

        for (size_t pi = 0; pi < passCount; ++pi)
        {
            const auto& pass = m_passes[pi];
            for (const auto& binding : pass.resources)
            {
                if (binding.usage == RGResourceUsage::Read)
                {
                    int writer = latestWriterForResource[binding.handle.id];
                    if (writer >= 0 && static_cast<size_t>(writer) != pi)
                    {
                        passEdges[writer].push_back(pi);
                        inDegree[pi]++;
                    }
                }
                else
                {
                    latestWriterForResource[binding.handle.id] = static_cast<int>(pi);
                }
            }
        }

        std::vector<size_t> queue;
        for (size_t i = 0; i < passCount; ++i)
            if (inDegree[i] == 0)
                queue.push_back(i);

        size_t front = 0;
        while (front < queue.size())
        {
            size_t cur = queue[front++];
            m_sortedPassIndices.push_back(cur);
            for (size_t next : passEdges[cur])
            {
                if (--inDegree[next] == 0)
                    queue.push_back(next);
            }
        }

        if (m_sortedPassIndices.size() != passCount)
        {
            Logger::Log("RenderGraph::Compile() -- circular dependency detected! Aborting.", LogLevel::Error);
            return false;
        }

        std::vector<int> firstUse(resCount, -1);
        std::vector<int> lastUse(resCount, -1);

        for (size_t si = 0; si < m_sortedPassIndices.size(); ++si)
        {
            size_t pi = m_sortedPassIndices[si];
            for (const auto& binding : m_passes[pi].resources)
            {
                if (firstUse[binding.handle.id] < 0) firstUse[binding.handle.id] = static_cast<int>(si);
                lastUse[binding.handle.id] = static_cast<int>(si);
            }
        }

        auto* pool = TransientRTPool::Get();

        for (size_t rid = 0; rid < resCount; ++rid)
        {
            auto& res = m_resources[rid];
            if (res.kind == RGResourceKind::Persistent)
            {
                m_physicalRTs[rid] = RCPtr<RenderTexture>::UnsafeCreate(res.external->GetObjectHandle());
                m_handleToRTIndex[static_cast<uint32_t>(rid)] = rid;
            }
            else
            {
                if (pool && firstUse[rid] >= 0)
                {
                    auto rt = pool->Acquire(res.texDesc);
                    m_physicalRTs[rid] = std::move(rt);
                    m_handleToRTIndex[static_cast<uint32_t>(rid)] = rid;
                    m_acquiredTransients.push_back({ res.texDesc, rid });
                }
            }
        }

        m_compiled = true;
        return true;
    }

    void RenderGraph::Execute(gfx::GFXCommandBuffer& cmd)
    {
        if (!m_compiled)
        {
            Logger::Log("RenderGraph::Execute() called without Compile()!", LogLevel::Error);
            return;
        }

        for (size_t si : m_sortedPassIndices)
        {
            const auto& pass = m_passes[si];
            if (!pass.executeFunc) continue;

            // --- barrier transitions ---
            for (const auto& binding : pass.resources)
            {
                auto it = m_handleToRTIndex.find(binding.handle.id);
                if (it == m_handleToRTIndex.end()) continue;

                auto* rt = m_physicalRTs[it->second].GetPtr();
                if (!rt) continue;

                const gfx::GFXResourceLayout barrierLayout = (binding.usage == RGResourceUsage::Read)
                    ? gfx::GFXResourceLayout::ShaderReadOnly
                    : gfx::GFXResourceLayout::RenderTarget;

                for (const auto& gfxTex : rt->GetRenderTargets())
                {
                    if (!gfxTex) continue;
                    auto view = gfxTex->Get2DView(0);
                    if (view)
                        cmd.CmdImageTransitionBarrier(view.get(), barrierLayout);
                }
            }

            // --- find the first Write RT to bind as framebuffer ---
            gfx::GFXFrameBufferObject* fbo      = nullptr;
            const RGAttachmentDesc*    fboAttach = nullptr;
            for (const auto& binding : pass.resources)
            {
                if (binding.usage != RGResourceUsage::Write) continue;
                auto it = m_handleToRTIndex.find(binding.handle.id);
                if (it == m_handleToRTIndex.end()) continue;
                auto* rt = m_physicalRTs[it->second].GetPtr();
                if (!rt) continue;
                fbo       = rt->GetGfxFrameBufferObject().get();
                fboAttach = &binding.attachment;
                break;
            }

            RGPassContext ctx(m_physicalRTs, m_handleToRTIndex, pass.perPassResources);

            // Prepare: runs after Compile, before BeginRenderPass.
            // Safe place for GPU resource creation, material preparation, descriptor set updates.
            if (pass.prepareFunc)
                pass.prepareFunc(ctx);

            const bool beginPass = fbo && fboAttach && !pass.noRenderPass;

            cmd.CmdPushDebugInfo(pass.name);
            if (beginPass)
            {
                cmd.SetFrameBuffer(fbo);
                cmd.CmdBeginRenderPass(pass.name, *fboAttach);
            }

            pass.executeFunc(ctx, cmd);

            if (beginPass)
            {
                cmd.CmdEndRenderPass();
                cmd.SetFrameBuffer(nullptr);
            }
            cmd.CmdPopDebugInfo();
        }

        auto* pool = TransientRTPool::Get();
        if (pool)
        {
            for (auto& entry : m_acquiredTransients)
            {
                auto& rtRC = m_physicalRTs[entry.rtIndex];
                if (rtRC)
                    pool->Release(entry.desc, std::move(rtRC));
            }
        }
        m_acquiredTransients.clear();
        m_compiled = false;
    }

    void RenderGraph::Reset()
    {
        auto* pool = TransientRTPool::Get();
        if (pool)
        {
            for (auto& entry : m_acquiredTransients)
            {
                auto& rtRC = m_physicalRTs[entry.rtIndex];
                if (rtRC)
                    pool->Release(entry.desc, std::move(rtRC));
            }
        }
        m_acquiredTransients.clear();
        m_compiled = false;
    }
}
