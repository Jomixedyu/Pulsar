#include "TransientRTPool.h"
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

    RCPtr<RenderTexture> TransientRTPool::Acquire(const RGTextureDesc& desc)
    {
        auto& bucket = m_free[desc];
        if (!bucket.empty())
        {
            auto entry = std::move(bucket.back());
            bucket.pop_back();
            return entry.rt;
        }

        static uint32_t s_counter = 0;
        index_string name = index_string{std::string("_TransientRT_") + std::to_string(++s_counter)};
        auto rt = RenderTexture::StaticCreate(name, desc.Width, desc.Height, desc.TargetInfos);
        return rt;
    }

    void TransientRTPool::Release(const RGTextureDesc& desc, RCPtr<RenderTexture> rt)
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
                        DestroyObject(e.rt);
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
                DestroyObject(entry.rt);
        }
        m_free.clear();
    }
}
