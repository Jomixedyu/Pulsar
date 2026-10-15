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
        int32_t     Depth   = 0;   // nesting depth (0 = top level)
        uint32_t    Color   = 0;   // 0 = auto (hashed from name)

        float DurationMs() const { return EndMs - StartMs; }
    };

    // One fully captured frame: the flattened list of scopes plus the total frame time.
    struct ProfileFrame
    {
        std::vector<ProfileEntry> Entries;
        float                     TotalMs = 0.f;
        uint64_t                  Index   = 0; // monotonic frame counter
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

        void BeginFrame();
        void EndFrame();

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

        struct OpenScope
        {
            size_t EntryIndex; // index into m_current.Entries
        };

        float NowMs() const;

        bool                      m_enabled = true;
        bool                      m_frozen  = false;
        Clock::time_point         m_frameStart{};
        uint64_t                  m_frameCounter = 0;
        ProfileFrame              m_current;
        std::vector<OpenScope>    m_openStack;
        std::vector<ProfileFrame> m_history;
    };

    // RAII scope. Prefer the PROFILE_SCOPE macro.
    class ProfileScope
    {
    public:
        explicit ProfileScope(std::string_view name, uint32_t color = 0)
        {
            Profiler::Get().PushScope(name, color);
        }
        ~ProfileScope() { Profiler::Get().PopScope(); }

        ProfileScope(const ProfileScope&) = delete;
        ProfileScope& operator=(const ProfileScope&) = delete;
    };
} // namespace pulsar

#define PROFILE_CONCAT_INNER(a, b) a##b
#define PROFILE_CONCAT(a, b) PROFILE_CONCAT_INNER(a, b)

// Times the enclosing block. Name must be a string literal or string_view.
#define PROFILE_SCOPE(name) ::pulsar::ProfileScope PROFILE_CONCAT(_profScope_, __LINE__){name}
#define PROFILE_SCOPE_COLOR(name, color) ::pulsar::ProfileScope PROFILE_CONCAT(_profScope_, __LINE__){name, color}
