#pragma once
#include <Pulsar/Assets/RenderTexture.h>
#include <corelib/CommonException.h>
#include <unordered_map>
#include <vector>

namespace pulsar
{
    // Describes the dimensions and format of a transient render texture
    struct RGTextureDesc
    {
        int32_t Width = 0;
        int32_t Height = 0;
        array_list<RenderTargetInfo> TargetInfos;

        bool operator==(const RGTextureDesc& o) const
        {
            if (Width != o.Width || Height != o.Height) return false;
            if (TargetInfos.size() != o.TargetInfos.size()) return false;
            for (size_t i = 0; i < TargetInfos.size(); ++i)
            {
                if (TargetInfos[i].Format != o.TargetInfos[i].Format) return false;
                if (TargetInfos[i].TargetType != o.TargetInfos[i].TargetType) return false;
            }
            return true;
        }
    };

    struct RGTextureDescHash
    {
        size_t operator()(const RGTextureDesc& d) const noexcept
        {
            size_t h = std::hash<int32_t>{}(d.Width) ^ (std::hash<int32_t>{}(d.Height) << 16);
            for (auto& t : d.TargetInfos)
            {
                h ^= std::hash<int32_t>{}(static_cast<int32_t>(t.Format)) * 2654435761u;
                h ^= std::hash<int32_t>{}(static_cast<int32_t>(t.TargetType)) * 40503u;
            }
            return h;
        }
    };

    class TransientRTPool;

    // RAII wrapper: auto-releases the RT back to the pool on destruction
    class ScopedRT
    {
    public:
        ScopedRT() = default;
        ScopedRT(TransientRTPool* pool, RCPtr<RenderTexture> rt, const RGTextureDesc& desc)
            : m_pool(pool), m_rt(std::move(rt)), m_desc(desc) {}

        ScopedRT(const ScopedRT&) = delete;
        ScopedRT& operator=(const ScopedRT&) = delete;

        ScopedRT(ScopedRT&& o) noexcept
            : m_pool(o.m_pool), m_rt(std::move(o.m_rt)), m_desc(std::move(o.m_desc))
        {
            o.m_pool = nullptr;
        }

        ScopedRT& operator=(ScopedRT&& o) noexcept
        {
            Release();
            m_pool = o.m_pool; m_rt = std::move(o.m_rt); m_desc = std::move(o.m_desc);
            o.m_pool = nullptr;
            return *this;
        }

        ~ScopedRT() { Release(); }

        RenderTexture* Get() const { return m_rt.GetPtr(); }
        RCPtr<RenderTexture> GetRC() const { return m_rt; }
        explicit operator bool() const { return m_rt != nullptr; }

    private:
        void Release();
        TransientRTPool* m_pool = nullptr;
        RCPtr<RenderTexture> m_rt;
        RGTextureDesc m_desc;
    };

    // Global pool for transient render textures.
    // Buckets RTs by (width, height, format) and reuses them across frames.
    class TransientRTPool
    {
    public:
        static TransientRTPool* Get();
        static void Initialize();
        static void Shutdown();

        // Acquire an RT matching desc. Returns from free list or creates new.
        RCPtr<RenderTexture> Acquire(const RGTextureDesc& desc);

        // Return an RT to the pool's free list.
        void Release(const RGTextureDesc& desc, RCPtr<RenderTexture> rt);

        // Acquire with RAII wrapper
        ScopedRT AcquireScoped(const RGTextureDesc& desc);

        // Destroy RTs that have not been used for more than gcFrameThreshold frames.
        void TickGC(uint64_t currentFrame, uint32_t gcFrameThreshold = 8);

        // Destroy ALL RTs held by the pool (called on engine shutdown).
        void DestroyAll();

    private:
        struct PoolEntry
        {
            RCPtr<RenderTexture> rt;
            uint64_t lastUsedFrame = 0;
        };

        // free[desc] = list of idle RTs for that desc
        std::unordered_map<RGTextureDesc, std::vector<PoolEntry>, RGTextureDescHash> m_free;

        static TransientRTPool* s_instance;
    };

    // ---- ScopedRT inline impl ----
    inline void ScopedRT::Release()
    {
        if (m_pool && m_rt)
        {
            m_pool->Release(m_desc, std::move(m_rt));
            m_pool = nullptr;
        }
    }
}
