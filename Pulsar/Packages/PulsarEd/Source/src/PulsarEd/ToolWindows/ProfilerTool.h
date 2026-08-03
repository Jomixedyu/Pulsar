#pragma once

#include <PulsarEd/Assembly.h>
#include <PulsarEd/Windows/ToolWindow.h>

#include <chrono>
#include <cstdint>
#include <memory>

namespace pulsar
{
    struct ProfileFrame;
}

namespace ImGuiUtils
{
    class ProfilerGraph;
}

namespace pulsared
{
    class ProfilerTool : public ToolWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ProfilerTool, ToolWindow);

    public:
        ProfilerTool();
        ~ProfilerTool() override;
        string_view GetWindowDisplayName() const override { return ICON_FK_BAR_CHART " Profiler"; }
        void OnDrawImGui(float dt) override;

    private:
        void SyncLegitGraph();
        void UpdateAverageStats(const pulsar::ProfileFrame& frame);
        void DrawLegitGraph(const pulsar::ProfileFrame& frame);
        void DrawFlameChart(const pulsar::ProfileFrame& frame);
        void DrawEntryTable(const pulsar::ProfileFrame& frame);

        std::unique_ptr<ImGuiUtils::ProfilerGraph> m_cpuGraph;
        uint64_t m_lastLoadedFrameIndex = UINT64_MAX;
        int m_frameOffset = 0;
        int m_frameWidth = 3;
        int m_frameSpacing = 1;
        float m_maxFrameTimeMs = 33.333f;
        float m_flameZoom = 1.0f;
        bool m_useColoredLegendText = true;
        bool m_showFlameChart = true;
        bool m_showTable = true;
        uint64_t m_lastStatsFrameIndex = UINT64_MAX;
        uint32_t m_statsFrameCount = 0;
        float m_statsFrameTimeSumMs = 0.0f;
        float m_displayFps = 0.0f;
        float m_displayFrameMs = 0.0f;
        std::chrono::steady_clock::time_point m_lastStatsUpdate{};
    };
}