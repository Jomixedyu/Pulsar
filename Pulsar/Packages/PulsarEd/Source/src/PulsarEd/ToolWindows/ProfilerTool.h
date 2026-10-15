#pragma once

#include <PulsarEd/Assembly.h>
#include <PulsarEd/Windows/ToolWindow.h>

#include <chrono>
#include <cstdint>

namespace pulsar
{
    struct ProfileFrame;
}

namespace pulsared
{
    class ProfilerTool : public ToolWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ProfilerTool, ToolWindow);

    public:
        ProfilerTool();
        ~ProfilerTool() override = default;
        string_view GetWindowDisplayName() const override { return ICON_FK_BAR_CHART " Profiler"; }
        void OnDrawImGui(float dt) override;

    private:
        const pulsar::ProfileFrame* GetSelectedFrame();
        void DrawFrameStrip();
        void DrawTimeline(const pulsar::ProfileFrame& frame);
        void DrawEntryTable(const pulsar::ProfileFrame& frame);
        void UpdateAverageStats(const pulsar::ProfileFrame& frame);

        uint64_t m_selectedFrame = UINT64_MAX;  // by frame index, stable across ring shifts
        bool m_followLatest = true;
        bool m_showTable = true;
        float m_zoom = 1.0f;

        uint64_t m_lastStatsFrameIndex = UINT64_MAX;
        uint32_t m_statsFrameCount = 0;
        float m_statsFrameTimeSumMs = 0.0f;
        float m_displayFps = 0.0f;
        float m_displayFrameMs = 0.0f;
        std::chrono::steady_clock::time_point m_lastStatsUpdate{};
    };
}