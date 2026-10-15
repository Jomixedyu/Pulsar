#include "Profiler.h"
#include <utility>

namespace pulsar
{
    // Deterministic pastel-ish color from a name, used when an entry has no explicit color.
    static uint32_t HashColor(std::string_view name)
    {
        uint32_t h = 2166136261u;
        for (char c : name)
        {
            h ^= static_cast<uint8_t>(c);
            h *= 16777619u;
        }
        // Bias toward brighter colors so bars are readable on a dark theme.
        uint8_t r = 96 + (h & 0x7F);
        uint8_t g = 96 + ((h >> 8) & 0x7F);
        uint8_t b = 96 + ((h >> 16) & 0x7F);
        return 0xFF000000u | (static_cast<uint32_t>(b) << 16) | (static_cast<uint32_t>(g) << 8) | r;
    }

    Profiler& Profiler::Get()
    {
        static Profiler s_instance;
        return s_instance;
    }

    float Profiler::NowMs() const
    {
        return std::chrono::duration<float, std::chrono::milliseconds::period>(Clock::now() - m_frameStart).count();
    }

    void Profiler::BeginFrame()
    {
        if (!m_enabled || m_frozen)
            return;

        m_frameStart = Clock::now();
        m_current.Entries.clear();
        m_current.TotalMs = 0.f;
        m_current.Index   = m_frameCounter;
        m_openStack.clear();
    }

    void Profiler::EndFrame()
    {
        if (!m_enabled || m_frozen)
            return;

        m_current.TotalMs = NowMs();
        ++m_frameCounter;

        m_history.push_back(m_current);
        if (m_history.size() > static_cast<size_t>(kHistorySize))
        {
            m_history.erase(m_history.begin());
        }
    }

    void Profiler::PushScope(std::string_view name, uint32_t color)
    {
        if (!m_enabled || m_frozen)
            return;

        ProfileEntry entry;
        entry.Name    = std::string(name);
        entry.StartMs = NowMs();
        entry.Depth   = static_cast<int32_t>(m_openStack.size());
        entry.Color   = color != 0 ? color : HashColor(name);

        m_openStack.push_back({m_current.Entries.size()});
        m_current.Entries.push_back(std::move(entry));
    }

    void Profiler::PopScope()
    {
        if (!m_enabled || m_frozen || m_openStack.empty())
            return;

        const size_t idx = m_openStack.back().EntryIndex;
        m_openStack.pop_back();
        m_current.Entries[idx].EndMs = NowMs();
    }

    const ProfileFrame* Profiler::GetLatestFrame() const
    {
        if (m_history.empty())
            return nullptr;
        return &m_history.back();
    }
} // namespace pulsar
