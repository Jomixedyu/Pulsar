#pragma once

#include <chrono>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

namespace pulsar
{
    // A single timed scope inside one frame. Times are milliseconds relative to the
    // frame's BeginFrame() call, so a UI can lay them out on a shared timeline.
    struct ProfileEntry
    {
        std::string Name;
        float       StartMs = 0.f; // start, relative to frame begin
        float       EndMs   = 0.f; // end,   relative to frame begin
        int32_t     Depth   = 0;   // nesting depth within its track (0 = top level)
        uint32_t    Color   = 0;   // 0 = auto (hashed from name)
        uint32_t    Track   = 0;   // thread lane (0 = main thread)

        float DurationMs() const { return EndMs - StartMs; }
    };

    // One fully captured frame: the flattened list of scopes plus the total frame time.
    struct ProfileFrame
    {
        std::vector<ProfileEntry> Entries;
        float                     MainMs  = 0.f; // main-thread tick time (frame pacing)
        float                     TotalMs = 0.f; // whole pipeline span incl. render tail
        uint64_t                  Index   = 0;   // monotonic frame counter
    };

    // Lightweight hierarchical CPU profiler.
    //
    // Usage:
    //   Profiler::Get().BeginFrame();
    //   { PROFILE_SCOPE("AppLoop"); ... }   // RAII, nesting is tracked automatically
    //   Profiler::Get().EndFrame();
    //
    // Every frame is flattened into a ProfileFrame and pushed into a ring buffer of the
    // last History() frames. Capture can be paused (Freeze) so a UI can inspect a stable
    // snapshot while comparing before/after an optimization.
    class Profiler
    {
    public:
        static Profiler& Get();

        static constexpr int32_t kHistorySize = 240;
        static constexpr uint32_t kMainTrack = 0;

        // Frame boundaries; call on every profiled thread. Each track counts its own
        // frames; frame N on any track pairs with frame N on the others (one render
        // frame is dispatched per main tick, so counters stay aligned).
        void BeginFrame();
        void EndFrame();

        // Register a named thread lane; returns its index. Idempotent by name.
        // Does NOT bind the calling thread -- bind explicitly with BindTrack.
        uint32_t RegisterTrack(std::string_view name);
        // Route this thread's PROFILE_SCOPE calls to a track. Call once on the
        // owning thread (the main thread is always track 0 by default).
        void BindTrack(uint32_t track);
        uint32_t GetTrackIndex(std::string_view name) const;
        const std::vector<std::string>& GetTrackNames() const { return m_trackNames; }

        // Called by ProfileScope; not meant for direct use.
        void PushScope(std::string_view name, uint32_t color);
        void PopScope();

        bool IsEnabled() const { return m_enabled; }
        void SetEnabled(bool v) { m_enabled = v; }

        // When frozen, BeginFrame/EndFrame/Push/Pop become no-ops so the last captured
        // history stays stable for inspection.
        bool IsFrozen() const { return m_frozen; }
        void SetFrozen(bool v) { m_frozen = v; }

        // Ring buffer access (newest last). May be empty before the first frame.
        const std::vector<ProfileFrame>& GetHistory() const { return m_history; }
        const ProfileFrame*              GetLatestFrame() const;

    private:
        using Clock = std::chrono::high_resolution_clock;

        struct Track
        {
            // Recording state (only touched on the owning thread).
            std::vector<ProfileEntry> Entries;
            std::vector<size_t>       OpenStack;
            Clock::time_point         Epoch{};
            uint64_t                  Index = 0;
            uint64_t                  FrameCounter = 0;  // this track's own frame count
            bool                      Recording = false;

            // Finalized, awaiting merge on the main thread.
            std::vector<ProfileEntry> PendingEntries;
            uint64_t                  PendingIndex = 0;
            bool                      HasPending = false;
        };

        float NowMs(Clock::time_point epoch) const;

        bool                      m_enabled = false;  // off by default; toggle from the profiler UI
        bool                      m_frozen  = false;
        Clock::time_point         m_epoch{};      // current main frame start
        Clock::time_point         m_epochs[8]{};  // recent frame epochs, indexed by frameIndex % 8
        uint64_t                  m_frameIndex = 0;
        std::vector<Track>        m_tracks;
        std::vector<std::string>  m_trackNames;

        // Main half of the previous frame, held one frame so the render half (recorded
        // concurrently during the next game tick) can merge into the same ProfileFrame.
        ProfileFrame              m_pendingMain;
        bool                      m_hasPendingMain = false;
        std::vector<ProfileFrame> m_history;

        static thread_local uint32_t t_trackIndex;
    };

    // RAII scope. Prefer the PROFILE_SCOPE macro.
    class ProfileScope
    {
    public:
        explicit ProfileScope(std::string_view name, uint32_t color = 0)
        {
            // Gate here so a disabled profiler costs one branch per scope (no call,
            // no string copy, no entry push).
            if (Profiler::Get().IsEnabled())
            {
                Profiler::Get().PushScope(name, color);
                m_active = true;
            }
        }
        ~ProfileScope() { if (m_active) Profiler::Get().PopScope(); }

        ProfileScope(const ProfileScope&) = delete;
        ProfileScope& operator=(const ProfileScope&) = delete;

    private:
        bool m_active = false;
    };
} // namespace pulsar

#define PROFILE_CONCAT_INNER(a, b) a##b
#define PROFILE_CONCAT(a, b) PROFILE_CONCAT_INNER(a, b)

// Times the enclosing block. Name must be a string literal or string_view.
#define PROFILE_SCOPE(name) ::pulsar::ProfileScope PROFILE_CONCAT(_profScope_, __LINE__){name}
#define PROFILE_SCOPE_COLOR(name, color) ::pulsar::ProfileScope PROFILE_CONCAT(_profScope_, __LINE__){name, color}
