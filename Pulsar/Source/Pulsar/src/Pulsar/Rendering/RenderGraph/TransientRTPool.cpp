#include "TransientRTPool.h"
#include <Pulsar/Application.h>
#include <Pulsar/Logger.h>

namespace pulsar
{
    TransientRTPool* TransientRTPool::s_instance = nullptr;

    TransientRTPool* TransientRTPool::Get()
    {
        return s_instance;
    }

    void TransientRTPool::Initialize()
    {
        assert(!s_instance);
        s_instance = new TransientRTPool();
    }

    void TransientRTPool::Shutdown()
    {
        if (s_instance)
        {
            s_instance->DestroyAll();
            delete s_instance;
            s_instance = nullptr;
        }
    }

    std::shared_ptr<RGPhysicalTexture> TransientRTPool::Acquire(const RGTextureDesc& desc)
    {
        auto& bucket = m_free[desc];
        while (!bucket.empty())
        {
            auto entry = std::move(bucket.back());
            bucket.pop_back();

            bool valid = false;
            if (entry.rt && !entry.rt->attachments.empty())
            {
                auto& gfxTex = entry.rt->attachments[0];
                if (gfxTex && gfxTex->GetSamplerConfig().Filter == gfx::GFXSamplerFilter::Linear)
                    valid = true;
            }

            if (valid)
                return entry.rt;

            // invalid or stale entry: drop it (GPU resources released via shared_ptr dtor)
        }

        auto gfx = Application::GetGfxApp();
        if (!gfx)
            return nullptr;

        auto pt = std::make_shared<RGPhysicalTexture>();
        pt->width  = desc.Width;
        pt->height = desc.Height;

        gfx::GFXSamplerConfig samplerCfg{};
        samplerCfg.Filter = gfx::GFXSamplerFilter::Linear;
        samplerCfg.AddressMode = gfx::GFXSamplerAddressMode::ClampToEdge;

        uint32_t sampleCount = 1;
        for (auto& info : desc.TargetInfos)
        {
            sampleCount = info.SampleCount;
            auto tex = gfx->CreateRenderTarget(
                desc.Width, desc.Height,
                info.TargetType,
                info.Format,
                samplerCfg,
                info.SampleCount,
                info.IsTransientAttachment);
            pt->attachments.push_back(tex);
        }

        if (!pt->attachments.empty())
        {
            array_list<gfx::GFXTexture2DView_sp> views;
            for (auto& att : pt->attachments)
                views.push_back(att->Get2DView(0));
            pt->fbo = gfx->CreateFrameBufferObject(views);
        }

        return pt;
    }

    void TransientRTPool::Release(const RGTextureDesc& desc, std::shared_ptr<RGPhysicalTexture> rt)
    {
        if (!rt) return;
        auto& bucket = m_free[desc];
        PoolEntry entry;
        entry.rt = std::move(rt);
        entry.lastUsedFrame = 0;
        bucket.push_back(std::move(entry));
    }

    ScopedRT TransientRTPool::AcquireScoped(const RGTextureDesc& desc)
    {
        return ScopedRT(this, Acquire(desc), desc);
    }

    void TransientRTPool::TickGC(uint64_t currentFrame, uint32_t gcFrameThreshold)
    {
        for (auto& [desc, bucket] : m_free)
        {
            for (auto& entry : bucket)
            {
                if (entry.lastUsedFrame == 0)
                    entry.lastUsedFrame = currentFrame;
            }

            bucket.erase(
                std::remove_if(bucket.begin(), bucket.end(), [&](PoolEntry& e) {
                    if (currentFrame - e.lastUsedFrame >= gcFrameThreshold)
                    {
                        // GPU resources released when shared_ptr drops to zero
                        e.rt.reset();
                        return true;
                    }
                    return false;
                }),
                bucket.end()
            );
        }

        for (auto it = m_free.begin(); it != m_free.end();)
        {
            if (it->second.empty())
                it = m_free.erase(it);
            else
                ++it;
        }
    }

    void TransientRTPool::DestroyAll()
    {
        for (auto& [desc, bucket] : m_free)
        {
            for (auto& entry : bucket)
                entry.rt.reset();
        }
        m_free.clear();
    }
}
