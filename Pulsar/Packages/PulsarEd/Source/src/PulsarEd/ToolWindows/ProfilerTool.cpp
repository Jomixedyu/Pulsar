#include <PulsarEd/ToolWindows/ProfilerTool.h>

#include <Pulsar/Profiler.h>

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdint>
#include <sstream>
#include <vector>

namespace glm
{
    struct vec2
    {
        float x = 0.0f;
        float y = 0.0f;

        vec2() = default;
        vec2(float x, float y) : x(x), y(y) {}
    };

    inline vec2 operator+(const vec2& lhs, const vec2& rhs)
    {
        return {lhs.x + rhs.x, lhs.y + rhs.y};
    }
}

#include <ImGuiProfilerRenderer.h>

namespace pulsared
{
    ProfilerTool::ProfilerTool()
        : m_cpuGraph(std::make_unique<ImGuiUtils::ProfilerGraph>(Profiler::kHistorySize))
    {
        m_winSize = {900, 620};
        m_lastStatsUpdate = std::chrono::steady_clock::now();
    }

    ProfilerTool::~ProfilerTool() = default;

    static const pulsar::ProfileFrame* GetSelectedFrame(int& frameOffset)
    {
        const auto& history = Profiler::Get().GetHistory();
        if (history.empty())
            return nullptr;

        const int maxOffset = static_cast<int>(history.size()) - 1;
        frameOffset = std::clamp(frameOffset, 0, maxOffset);
        return &history[history.size() - 1 - static_cast<size_t>(frameOffset)];
    }

    static legit::ProfilerTask ToLegitTask(const ProfileEntry& entry)
    {
        legit::ProfilerTask task{};
        task.startTime = static_cast<double>(entry.StartMs) / 1000.0;
        task.endTime = static_cast<double>(std::max(entry.EndMs, entry.StartMs)) / 1000.0;
        task.name = entry.Name;
        task.color = entry.Color;
        return task;
    }

    void ProfilerTool::SyncLegitGraph()
    {
        const auto& history = Profiler::Get().GetHistory();
        if (history.empty())
            return;

        size_t firstFrame = 0;
        if (m_lastLoadedFrameIndex != UINT64_MAX)
        {
            auto it = std::ranges::find_if(history, [this](const ProfileFrame& frame) {
                return frame.Index > m_lastLoadedFrameIndex;
            });
            firstFrame = it == history.end() ? history.size() : static_cast<size_t>(std::distance(history.begin(), it));
        }

        for (size_t frameIndex = firstFrame; frameIndex < history.size(); ++frameIndex)
        {
            const auto& frame = history[frameIndex];
            std::vector<legit::ProfilerTask> tasks;
            tasks.reserve(frame.Entries.size() + 1);

            if (frame.Entries.empty())
            {
                legit::ProfilerTask idle{};
                idle.startTime = 0.0;
                idle.endTime = static_cast<double>(frame.TotalMs) / 1000.0;
                idle.name = "Frame";
                idle.color = IM_COL32(80, 80, 90, 255);
                tasks.push_back(std::move(idle));
            }
            else
            {
                for (const auto& entry : frame.Entries)
                {
                    if (entry.Depth == 0)
                    {
                        tasks.push_back(ToLegitTask(entry));
                    }
                }
            }

            m_cpuGraph->LoadFrameData(tasks.data(), tasks.size());
            m_lastLoadedFrameIndex = frame.Index;
        }
    }

    void ProfilerTool::OnDrawImGui(float dt)
    {
        auto& profiler = Profiler::Get();

        bool enabled = profiler.IsEnabled();
        if (ImGui::Checkbox("Enabled", &enabled))
        {
            profiler.SetEnabled(enabled);
        }

        ImGui::SameLine();
        bool frozen = profiler.IsFrozen();
        if (ImGui::Checkbox("Freeze", &frozen))
        {
            profiler.SetFrozen(frozen);
        }

        ImGui::SameLine();
        ImGui::Checkbox("Flame Chart", &m_showFlameChart);
        ImGui::SameLine();
        ImGui::Checkbox("Details", &m_showTable);

        SyncLegitGraph();

        const auto& history = profiler.GetHistory();
        if (history.empty())
        {
            ImGui::TextUnformatted("Waiting for captured frames...");
            return;
        }

        const auto* frame = GetSelectedFrame(m_frameOffset);
        if (!frame)
            return;

        UpdateAverageStats(*frame);
        DrawLegitGraph(*frame);

        if (m_showFlameChart)
        {
            DrawFlameChart(*frame);
        }

        if (m_showTable)
        {
            DrawEntryTable(*frame);
        }
    }


    void ProfilerTool::UpdateAverageStats(const pulsar::ProfileFrame& frame)
    {
        if (m_lastStatsFrameIndex != frame.Index)
        {
            m_lastStatsFrameIndex = frame.Index;
            ++m_statsFrameCount;
            m_statsFrameTimeSumMs += frame.TotalMs;
        }

        const auto now = std::chrono::steady_clock::now();
        const float elapsed = std::chrono::duration<float>(now - m_lastStatsUpdate).count();
        if (elapsed >= 1.0f && m_statsFrameCount > 0)
        {
            m_displayFps = static_cast<float>(m_statsFrameCount) / elapsed;
            m_displayFrameMs = m_statsFrameTimeSumMs / static_cast<float>(m_statsFrameCount);
            m_statsFrameCount = 0;
            m_statsFrameTimeSumMs = 0.0f;
            m_lastStatsUpdate = now;
        }
    }
    void ProfilerTool::DrawLegitGraph(const pulsar::ProfileFrame& frame)
    {
        ImGui::Text("Frame %llu  %.3f ms  Avg %.3f ms  %.1f FPS  %zu scopes",
            static_cast<unsigned long long>(frame.Index), frame.TotalMs, m_displayFrameMs, m_displayFps, frame.Entries.size());

        ImGui::SetNextItemWidth(180.0f);
        ImGui::SliderInt("Frame Offset", &m_frameOffset, 0, static_cast<int>(Profiler::Get().GetHistory().size()) - 1);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120.0f);
        ImGui::SliderFloat("Max Frame", &m_maxFrameTimeMs, 1.0f, 100.0f, "%.1f ms");
        ImGui::SameLine();
        ImGui::Checkbox("Colored Legend", &m_useColoredLegendText);

        ImGui::SetNextItemWidth(120.0f);
        ImGui::SliderInt("Frame Width", &m_frameWidth, 1, 8);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(120.0f);
        ImGui::SliderInt("Frame Spacing", &m_frameSpacing, 0, 4);

        m_cpuGraph->frameWidth = m_frameWidth;
        m_cpuGraph->frameSpacing = m_frameSpacing;
        m_cpuGraph->useColoredLegendText = m_useColoredLegendText;

        const ImVec2 avail = ImGui::GetContentRegionAvail();
        const int legendWidth = 240;
        const int graphWidth = std::max(100, static_cast<int>(avail.x) - legendWidth);
        const int graphHeight = std::clamp(static_cast<int>(avail.y * 0.25f), 120, 220);
        const float maxFrameTimeSeconds = std::max(0.001f, m_maxFrameTimeMs / 1000.0f);

        m_cpuGraph->RenderTimings(graphWidth, legendWidth, graphHeight, m_frameOffset, maxFrameTimeSeconds);
    }

    void ProfilerTool::DrawFlameChart(const pulsar::ProfileFrame& frame)
    {
        if (frame.Entries.empty())
            return;

        int maxDepth = 0;
        for (const auto& entry : frame.Entries)
        {
            maxDepth = std::max(maxDepth, entry.Depth);
        }

        const float rowHeight = 24.0f;
        const float rulerHeight = 6.0f;
        const float chartHeight = rulerHeight + static_cast<float>(maxDepth + 1) * rowHeight + 8.0f;
        const float visibleWidth = std::max(1.0f, ImGui::GetContentRegionAvail().x);
        m_flameZoom = std::clamp(m_flameZoom, 1.0f, 32.0f);
        const float chartWidth = visibleWidth * m_flameZoom;
        const float totalMs = std::max(frame.TotalMs, 0.001f);

        ImGui::SeparatorText("Flame Chart");
        ImGui::SetNextItemWidth(160.0f);
        ImGui::SliderFloat("Flame Zoom", &m_flameZoom, 1.0f, 32.0f, "%.1fx", ImGuiSliderFlags_Logarithmic);
        ImGui::SameLine();
        if (ImGui::SmallButton("Reset Zoom"))
        {
            m_flameZoom = 1.0f;
            ImGui::SetScrollX(0.0f);
        }

        ImGui::BeginChild("##ProfilerFlameChart", {0.0f, chartHeight + ImGui::GetStyle().ScrollbarSize}, true, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        const ImVec2 origin = ImGui::GetCursorScreenPos();
        ImGui::InvisibleButton("##ProfilerFlameChartCanvas", {chartWidth, chartHeight});

        auto* drawList = ImGui::GetWindowDrawList();
        const ImVec2 chartMax = {origin.x + chartWidth, origin.y + chartHeight};
        drawList->AddRectFilled(origin, chartMax, IM_COL32(18, 18, 22, 255));
        drawList->AddRect(origin, chartMax, IM_COL32(70, 70, 80, 255));

        for (int marker = 0; marker <= 4; ++marker)
        {
            const float t = static_cast<float>(marker) / 4.0f;
            const float x = origin.x + chartWidth * t;
            drawList->AddLine({x, origin.y}, {x, chartMax.y}, IM_COL32(48, 48, 56, 255));


        }

        for (int depth = 0; depth <= maxDepth; ++depth)
        {
            const float y = origin.y + rulerHeight + static_cast<float>(depth) * rowHeight;
            drawList->AddLine({origin.x, y}, {chartMax.x, y}, IM_COL32(36, 36, 42, 255));
        }

        for (const auto& entry : frame.Entries)
        {
            const float x0 = origin.x + chartWidth * (entry.StartMs / totalMs);
            const float x1 = origin.x + chartWidth * (entry.EndMs / totalMs);
            const float y0 = origin.y + rulerHeight + static_cast<float>(entry.Depth) * rowHeight + 3.0f;
            const float y1 = y0 + rowHeight - 6.0f;
            const ImVec2 rectMin = {x0, y0};
            const ImVec2 rectMax = {std::max(x0 + 2.0f, x1), y1};

            drawList->AddRectFilled(rectMin, rectMax, static_cast<ImU32>(entry.Color), 3.0f);
            drawList->AddRect(rectMin, rectMax, IM_COL32(255, 255, 255, 70), 3.0f);

            const float rectWidth = rectMax.x - rectMin.x;
            if (rectWidth > 32.0f)
            {
                char label[256];
                std::snprintf(label, sizeof(label), "%s %.2f ms", entry.Name.c_str(), entry.DurationMs());

                drawList->PushClipRect(rectMin, rectMax, true);
                drawList->AddText({rectMin.x + 4.0f, rectMin.y + 2.0f}, IM_COL32(20, 20, 24, 235), label);
                drawList->PopClipRect();
            }

            if (ImGui::IsMouseHoveringRect(rectMin, rectMax))
            {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(entry.Name.c_str());
                ImGui::Text("Depth: %d", entry.Depth);
                ImGui::Text("Start: %.3f ms", entry.StartMs);
                ImGui::Text("End: %.3f ms", entry.EndMs);
                ImGui::Text("Duration: %.3f ms", entry.DurationMs());
                ImGui::EndTooltip();
            }
        }

        ImGui::EndChild();
    }

    static size_t FindSubtreeEnd(const std::vector<ProfileEntry>& entries, size_t index)
    {
        const int32_t depth = entries[index].Depth;
        size_t end = index + 1;
        while (end < entries.size() && entries[end].Depth > depth)
        {
            ++end;
        }
        return end;
    }

    static float CalcDirectChildrenTime(const std::vector<ProfileEntry>& entries, size_t index, size_t subtreeEnd)
    {
        const int32_t childDepth = entries[index].Depth + 1;
        float childrenMs = 0.0f;
        for (size_t child = index + 1; child < subtreeEnd;)
        {
            if (entries[child].Depth == childDepth)
            {
                childrenMs += entries[child].DurationMs();
                child = FindSubtreeEnd(entries, child);
            }
            else
            {
                ++child;
            }
        }
        return childrenMs;
    }

    void ProfilerTool::DrawEntryTable(const pulsar::ProfileFrame& frame)
    {
        if (!ImGui::BeginTable("ProfilerHierarchy", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY, {0, 0}))
            return;

        ImGui::TableSetupColumn("Scope", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Total", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Self", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Frame", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableHeadersRow();

        const auto& entries = frame.Entries;
        std::vector<bool> openStack(static_cast<size_t>(32), true);

        for (size_t index = 0; index < entries.size(); ++index)
        {
            const auto& entry = entries[index];
            if (entry.Depth > 0 && (static_cast<size_t>(entry.Depth - 1) >= openStack.size() || !openStack[entry.Depth - 1]))
            {
                if (static_cast<size_t>(entry.Depth) >= openStack.size())
                    openStack.resize(static_cast<size_t>(entry.Depth + 8), false);
                openStack[entry.Depth] = false;
                continue;
            }

            if (static_cast<size_t>(entry.Depth) >= openStack.size())
                openStack.resize(static_cast<size_t>(entry.Depth + 8), true);

            const size_t subtreeEnd = FindSubtreeEnd(entries, index);
            const bool hasChildren = subtreeEnd > index + 1;
            const float totalMs = entry.DurationMs();
            const float selfMs = std::max(0.0f, totalMs - CalcDirectChildrenTime(entries, index, subtreeEnd));
            const float framePercent = frame.TotalMs > 0.0f ? totalMs / frame.TotalMs * 100.0f : 0.0f;

            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::PushID(static_cast<int>(index));
            ImGui::Indent(static_cast<float>(entry.Depth) * ImGui::GetTreeNodeToLabelSpacing());

            ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_SpanFullWidth;
            if (!hasChildren)
                flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            if (entry.Depth == 0)
                flags |= ImGuiTreeNodeFlags_DefaultOpen;

            const bool opened = ImGui::TreeNodeEx(entry.Name.c_str(), flags);
            openStack[entry.Depth] = hasChildren ? opened : true;

            ImGui::TableNextColumn();
            ImGui::Text("%.3f ms", totalMs);
            ImGui::TableNextColumn();
            ImGui::Text("%.3f ms", selfMs);
            ImGui::TableNextColumn();
            ImGui::Text("%.1f%%", framePercent);

            if (hasChildren && opened)
                ImGui::TreePop();
            ImGui::Unindent(static_cast<float>(entry.Depth) * ImGui::GetTreeNodeToLabelSpacing());
            ImGui::PopID();
        }

        ImGui::EndTable();
    }
}