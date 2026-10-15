#include "Profiler.h"
#include <utility>

namespace pulsar
{
    thread_local uint32_t Profiler::t_trackIndex = 0;

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

    float Profiler::NowMs(Clock::time_point epoch) const
    {
        return std::chrono::duration<float, std::chrono::milliseconds::period>(Clock::now() - epoch).count();
    }

    uint32_t Profiler::RegisterTrack(std::string_view name)
    {
        for (uint32_t i = 0; i < m_trackNames.size(); ++i)
        {
            if (m_trackNames[i] == name)
                return i;
        }
        m_trackNames.emplace_back(name);
        m_tracks.emplace_back();
        return static_cast<uint32_t>(m_tracks.size() - 1);
    }

    void Profiler::BindTrack(uint32_t track)
    {
        t_trackIndex = track;
    }

    uint32_t Profiler::GetTrackIndex(std::string_view name) const
    {
        for (uint32_t i = 0; i < m_trackNames.size(); ++i)
        {
            if (m_trackNames[i] == name)
                return i;
        }
        return kMainTrack;
    }

    void Profiler::BeginFrame()
    {
        // Counters and epochs advance even while disabled: tracks stay frame-aligned
        // and the epoch ring stays fresh, so enabling never records against stale time.
        if (m_frozen)
            return;

        if (m_tracks.empty())
        {
            m_trackNames.emplace_back("Main");
            m_tracks.emplace_back();
        }

        const uint32_t ti = t_trackIndex < m_tracks.size() ? t_trackIndex : kMainTrack;
        Track& t = m_tracks[ti];

        if (ti == kMainTrack)
        {
            m_epoch = Clock::now();
            m_epochs[m_frameIndex % 8] = m_epoch;
            t.Epoch = m_epoch;
            t.Index = m_frameIndex;
        }
        else
        {
            t.Index = t.FrameCounter;
            t.Epoch = m_epochs[t.Index % 8];
        }

        t.Entries.clear();
        t.OpenStack.clear();
        t.Recording = m_enabled;
    }

    void Profiler::EndFrame()
    {
        if (m_frozen || m_tracks.empty())
            return;

        const uint32_t ti = t_trackIndex < m_tracks.size() ? t_trackIndex : kMainTrack;
        Track& t = m_tracks[ti];
        t.Recording = false;

        if (!m_enabled)
        {
            // Counters still advance (see BeginFrame) so re-enabling stays aligned.
            if (ti == kMainTrack)
                ++m_frameIndex;
            ++t.FrameCounter;
            return;
        }

        if (ti == kMainTrack)
        {
            // The previous main frame's half is complete, and the render half of the
            // same frame (recorded concurrently during this tick) finalized before
            // this tick's WaitFrame. Merge and push.
            if (m_hasPendingMain)
            {
                ProfileFrame frame = std::move(m_pendingMain);
                m_hasPendingMain = false;

                for (uint32_t ti2 = 1; ti2 < m_tracks.size(); ++ti2)
                {
                    Track& track = m_tracks[ti2];
                    if (track.HasPending && track.PendingIndex == frame.Index)
                    {
                        for (auto& entry : track.PendingEntries)
                        {
                            frame.TotalMs = std::max(frame.TotalMs, entry.EndMs);
                            frame.Entries.push_back(std::move(entry));
                        }
                        track.PendingEntries.clear();
                        track.HasPending = false;
                    }
                }

                static int s_dumpLeft = 5;
                if (s_dumpLeft > 0)
                {
                    --s_dumpLeft;
                    float rStart = 1e9f, rEnd = 0;
                    for (auto& e : frame.Entries)
                        if (e.Track == 1) { rStart = std::min(rStart, e.StartMs); rEnd = std::max(rEnd, e.EndMs); }
                    fprintf(stderr, "[profdbg] frame %llu main=%.2f total=%.2f render=[%.2f..%.2f] entries=%zu\n",
                        (unsigned long long)frame.Index, frame.MainMs, frame.TotalMs, rStart, rEnd, frame.Entries.size());
                }
                m_history.push_back(std::move(frame));
                if (m_history.size() > static_cast<size_t>(kHistorySize))
                    m_history.erase(m_history.begin());
            }

            // Hold this frame's main half one tick; other tracks record the same
            // frame concurrently during the next tick and merge then.
            m_pendingMain.Index = m_frameIndex;
            m_pendingMain.MainMs = NowMs(m_epoch);
            m_pendingMain.TotalMs = m_pendingMain.MainMs;
            m_pendingMain.Entries = std::move(t.Entries);
            t.Entries.clear();
            m_hasPendingMain = true;

            ++m_frameIndex;
        }
        else
        {
            t.PendingEntries = std::move(t.Entries);
            t.Entries.clear();
            t.PendingIndex = t.Index;
            t.HasPending = true;
        }

        ++t.FrameCounter;
    }



    void Profiler::PushScope(std::string_view name, uint32_t color)
    {
        if (!m_enabled || m_frozen || m_tracks.empty())
            return;

        Track& t = m_tracks[t_trackIndex < m_tracks.size() ? t_trackIndex : kMainTrack];
        if (!t.Recording)
            return;

        ProfileEntry entry;
        entry.Name    = std::string(name);
        entry.StartMs = NowMs(t.Epoch);
        entry.Depth   = static_cast<int32_t>(t.OpenStack.size());
        entry.Color   = color != 0 ? color : HashColor(name);
        entry.Track   = t_trackIndex;

        t.OpenStack.push_back(t.Entries.size());
        t.Entries.push_back(std::move(entry));
    }

    void Profiler::PopScope()
    {
        if (!m_enabled || m_frozen || m_tracks.empty())
            return;

        Track& t = m_tracks[t_trackIndex < m_tracks.size() ? t_trackIndex : kMainTrack];
        if (!t.Recording || t.OpenStack.empty())
            return;

        const size_t idx = t.OpenStack.back();
        t.OpenStack.pop_back();
        t.Entries[idx].EndMs = NowMs(t.Epoch);
    }

    const ProfileFrame* Profiler::GetLatestFrame() const
    {
        if (m_history.empty())
            return nullptr;
        return &m_history.back();
    }
} // namespace pulsar
