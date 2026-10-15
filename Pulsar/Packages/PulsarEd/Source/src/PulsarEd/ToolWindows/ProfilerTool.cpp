#include <PulsarEd/ToolWindows/ProfilerTool.h>

#include <Pulsar/Profiler.h>

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace pulsared
{
    ProfilerTool::ProfilerTool()
    {
        m_winSize = {960, 640};
        m_lastStatsUpdate = std::chrono::steady_clock::now();
    }

    const pulsar::ProfileFrame* ProfilerTool::GetSelectedFrame()
    {
        const auto& history = Profiler::Get().GetHistory();
        if (history.empty())
            return nullptr;

        if (m_followLatest)
            return &history.back();

        for (const auto& frame : history)
        {
            if (frame.Index == m_selectedFrame)
                return &frame;
        }
        // selection fell off the ring; follow latest again
        m_followLatest = true;
        return &history.back();
    }

    void ProfilerTool::UpdateAverageStats(const pulsar::ProfileFrame& frame)
    {
        if (m_lastStatsFrameIndex != frame.Index)
        {
            m_lastStatsFrameIndex = frame.Index;
            ++m_statsFrameCount;
            m_statsFrameTimeSumMs += frame.MainMs;
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

    void ProfilerTool::OnDrawImGui(float dt)
    {
        auto& profiler = Profiler::Get();

        bool enabled = profiler.IsEnabled();
        if (ImGui::Checkbox("Enabled", &enabled))
            profiler.SetEnabled(enabled);
        ImGui::SameLine();
        bool frozen = profiler.IsFrozen();
        if (ImGui::Checkbox("Freeze", &frozen))
            profiler.SetFrozen(frozen);
        ImGui::SameLine();
        ImGui::Checkbox("Follow Latest", &m_followLatest);
        ImGui::SameLine();
        ImGui::Text("%.1f FPS  %.2f ms", m_displayFps, m_displayFrameMs);

        ImGui::Separator();

        DrawFrameStrip();

        const auto* frame = GetSelectedFrame();
        if (!frame)
        {
            ImGui::TextUnformatted("Waiting for captured frames...");
            return;
        }
        UpdateAverageStats(*frame);
        DrawTimeline(*frame);

        if (ImGui::CollapsingHeader("Details", ImGuiTreeNodeFlags_DefaultOpen))
            DrawEntryTable(*frame);
    }

    void ProfilerTool::DrawFrameStrip()
    {
        const auto& history = Profiler::Get().GetHistory();
        if (history.empty())
            return;

        // Fixed budget scale: one 30fps frame = full height; spikes clip instead of
        // squishing every other bar.
        const float maxMs = 33.333f;

        const float stripHeight = 56.0f;
        const ImVec2 avail = ImGui::GetContentRegionAvail();
        const float stripWidth = std::max(1.0f, avail.x);

        ImGui::BeginChild("##ProfilerFrameStrip", {stripWidth, stripHeight}, true);
        const ImVec2 origin = ImGui::GetCursorScreenPos();
        auto* drawList = ImGui::GetWindowDrawList();

        const float barWidth = 5.0f;
        const float spacing = 2.0f;
        const int visibleCount = static_cast<int>(stripWidth / (barWidth + spacing));
        const size_t begin = history.size() > static_cast<size_t>(visibleCount) ? history.size() - visibleCount : 0;

        for (size_t i = begin; i < history.size(); ++i)
        {
            const auto& frame = history[i];
            const float x0 = origin.x + static_cast<float>(i - begin) * (barWidth + spacing);
            const float h = stripHeight - 8.0f;
            const float barH = std::max(1.0f, h * frame.MainMs / maxMs);
            const ImVec2 bMin = {x0, origin.y + stripHeight - 4.0f - barH};
            const ImVec2 bMax = {x0 + barWidth, origin.y + stripHeight - 4.0f};

            const bool selected = !m_followLatest && frame.Index == m_selectedFrame;
            const ImU32 color = selected ? IM_COL32(240, 200, 80, 255) : IM_COL32(90, 140, 220, 255);
            drawList->AddRectFilled(bMin, bMax, color);

            if (ImGui::IsMouseHoveringRect(bMin, bMax))
            {
                ImGui::BeginTooltip();
                ImGui::Text("Frame %llu  %.2f ms", static_cast<unsigned long long>(frame.Index), frame.MainMs);
                ImGui::EndTooltip();
                if (ImGui::IsMouseClicked(ImGuiMouseButton_Left))
                {
                    m_selectedFrame = frame.Index;
                    m_followLatest = false;
                }
            }
        }
        ImGui::EndChild();
    }

    void ProfilerTool::DrawTimeline(const pulsar::ProfileFrame& frame)
    {
        const auto& trackNames = Profiler::Get().GetTrackNames();

        // Row layout: one band per track, scopes stacked by depth inside the band.
        std::vector<int> trackRowBase(trackNames.size(), 0);
        int totalRows = 0;
        for (size_t t = 0; t < trackNames.size(); ++t)
        {
            int maxDepth = -1;
            for (const auto& entry : frame.Entries)
            {
                if (entry.Track == t)
                    maxDepth = std::max(maxDepth, entry.Depth);
            }
            trackRowBase[t] = totalRows;
            // label row + depth rows
            totalRows += (maxDepth >= 0 ? maxDepth + 2 : 1);
        }

        float maxMs = std::max(frame.TotalMs, 0.001f);
        for (const auto& entry : frame.Entries)
            maxMs = std::max(maxMs, entry.EndMs);

        const float rowHeight = 22.0f;
        const float chartHeight = static_cast<float>(totalRows) * rowHeight + 12.0f;

        ImGui::SetNextItemWidth(140.0f);
        ImGui::SliderFloat("Zoom", &m_zoom, 1.0f, 16.0f, "%.1fx", ImGuiSliderFlags_Logarithmic);
        ImGui::SameLine();
        ImGui::Text("Frame %llu  main %.2f ms  pipeline %.2f ms", static_cast<unsigned long long>(frame.Index), frame.MainMs, frame.TotalMs);

        ImGui::BeginChild("##ProfilerTimeline", {0, chartHeight + ImGui::GetStyle().ScrollbarSize}, true,
            ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

        // Inner width (after the child's scrollbar/padding) is the zoom reference.
        const float visibleWidth = std::max(1.0f, ImGui::GetContentRegionAvail().x);
        const float chartWidth = std::max(visibleWidth, visibleWidth * m_zoom);
        const ImVec2 origin = ImGui::GetCursorScreenPos();

        // Wheel zooms horizontally, centered on the cursor: the time under the cursor
        // stays fixed in screen space (scroll correction applies next frame).
        if (ImGui::IsWindowHovered())
        {
            const float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.0f)
            {
                const float oldZoom = m_zoom;
                const float oldChartWidth = std::max(visibleWidth, visibleWidth * oldZoom);
                m_zoom = std::clamp(m_zoom * std::pow(1.2f, wheel), 1.0f, 16.0f);
                const float newChartWidth = std::max(visibleWidth, visibleWidth * m_zoom);
                const float mouseX = ImGui::GetIO().MousePos.x - origin.x;
                const float timeAtMouse = (ImGui::GetScrollX() + mouseX) / oldChartWidth;
                ImGui::SetScrollX(timeAtMouse * newChartWidth - mouseX);
            }
        }
        auto* drawList = ImGui::GetWindowDrawList();
        ImGui::InvisibleButton("##ProfilerTimelineCanvas", {chartWidth, chartHeight});

        const ImVec2 chartMax = {origin.x + chartWidth, origin.y + chartHeight};
        drawList->AddRectFilled(origin, chartMax, IM_COL32(16, 16, 20, 255));

        // time ruler
        for (int marker = 0; marker <= 8; ++marker)
        {
            const float t = static_cast<float>(marker) / 8.0f;
            const float x = origin.x + chartWidth * t;
            drawList->AddLine({x, origin.y}, {x, chartMax.y}, IM_COL32(48, 48, 56, 255));
            char label[32];
            std::snprintf(label, sizeof(label), "%.2f", maxMs * t);
            drawList->AddText({x + 2.0f, origin.y + 1.0f}, IM_COL32(120, 120, 130, 255), label);
        }

        for (size_t t = 0; t < trackNames.size(); ++t)
        {
            const float y0 = origin.y + static_cast<float>(trackRowBase[t]) * rowHeight;
            drawList->AddText({origin.x + 4.0f, y0 + 2.0f}, IM_COL32(200, 200, 210, 255), trackNames[t].c_str());
            const float bandBottom = origin.y + static_cast<float>(trackRowBase[t] + 1) * rowHeight;
            drawList->AddLine({origin.x, bandBottom}, {chartMax.x, bandBottom}, IM_COL32(70, 70, 80, 255));
        }

        for (const auto& entry : frame.Entries)
        {
            const uint32_t track = entry.Track < trackRowBase.size() ? entry.Track : 0;
            const float x0 = origin.x + chartWidth * (entry.StartMs / maxMs);
            const float x1 = origin.x + chartWidth * (entry.EndMs / maxMs);
            const float y0 = origin.y + static_cast<float>(trackRowBase[track] + 1 + entry.Depth) * rowHeight + 2.0f;
            const float y1 = y0 + rowHeight - 4.0f;

            const ImVec2 rMin = {x0, y0};
            const ImVec2 rMax = {std::max(x0 + 2.0f, x1), y1};
            drawList->AddRectFilled(rMin, rMax, static_cast<ImU32>(entry.Color), 2.0f);

            const float rectWidth = rMax.x - rMin.x;
            if (rectWidth > 34.0f)
            {
                char label[256];
                std::snprintf(label, sizeof(label), "%s %.2fms", entry.Name.c_str(), entry.DurationMs());
                drawList->PushClipRect(rMin, rMax, true);
                drawList->AddText({rMin.x + 3.0f, rMin.y + 3.0f}, IM_COL32(16, 16, 20, 255), label);
                drawList->PopClipRect();
            }

            if (ImGui::IsMouseHoveringRect(rMin, rMax))
            {
                ImGui::BeginTooltip();
                ImGui::TextUnformatted(entry.Name.c_str());
                ImGui::Text("Track: %s", entry.Track < trackNames.size() ? trackNames[entry.Track].c_str() : "?");
                ImGui::Text("Start: %.3f ms", entry.StartMs);
                ImGui::Text("Duration: %.3f ms", entry.DurationMs());
                ImGui::EndTooltip();
            }
        }

        ImGui::EndChild();
    }

    static size_t FindSubtreeEnd(const std::vector<pulsar::ProfileEntry>& entries, size_t index)
    {
        const int32_t depth = entries[index].Depth;
        const uint32_t track = entries[index].Track;
        size_t end = index + 1;
        while (end < entries.size() && entries[end].Track == track && entries[end].Depth > depth)
            ++end;
        return end;
    }

    void ProfilerTool::DrawEntryTable(const pulsar::ProfileFrame& frame)
    {
        const auto& trackNames = Profiler::Get().GetTrackNames();
        const auto& entries = frame.Entries;

        if (!ImGui::BeginTable("ProfilerEntries", 6,
                ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY,
                {0, ImGui::GetContentRegionAvail().y}))
            return;

        ImGui::TableSetupColumn("Scope", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Track", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableSetupColumn("Start", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        ImGui::TableSetupColumn("Total", ImGuiTableColumnFlags_WidthFixed, 90.0f);
        ImGui::TableSetupColumn("Depth", ImGuiTableColumnFlags_WidthFixed, 50.0f);
        ImGui::TableSetupColumn("Frame", ImGuiTableColumnFlags_WidthFixed, 70.0f);
        ImGui::TableHeadersRow();

        for (uint32_t track = 0; track < trackNames.size(); ++track)
        {
            // this track's entries form a contiguous block (merged in track order)
            size_t begin = entries.size();
            size_t end = entries.size();
            for (size_t i = 0; i < entries.size(); ++i)
            {
                if (entries[i].Track != track)
                    continue;
                if (begin == entries.size())
                    begin = i;
                end = i + 1;
            }
            if (begin == entries.size())
                continue;

            // Per-track total: this track's own busy span (first start to last end);
            // percentages are per-track.
            float trackStartMs = std::numeric_limits<float>::max();
            float trackEndMs = 0.0f;
            for (size_t i = begin; i < end; ++i)
            {
                trackStartMs = std::min(trackStartMs, entries[i].StartMs);
                trackEndMs = std::max(trackEndMs, entries[i].EndMs);
            }
            const float trackTotalMs = std::max(trackEndMs - trackStartMs, 0.001f);

            // Track section header
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(trackNames[track].c_str());
            ImGui::TableNextColumn();
            ImGui::TextDisabled("%.2f ms", trackTotalMs);
            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            ImGui::TableNextColumn();
            ImGui::TableNextColumn();

            std::vector<bool> openStack(32, true);
            for (size_t index = begin; index < end; ++index)
            {
                const auto& entry = entries[index];

                // hidden when any ancestor is collapsed
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
                ImGui::TextUnformatted(trackNames[track].c_str());
                ImGui::TableNextColumn();
                ImGui::Text("%.2f ms", entry.StartMs);
                ImGui::TableNextColumn();
                ImGui::Text("%.2f ms", totalMs);
                ImGui::TableNextColumn();
                ImGui::Text("%d", entry.Depth);
                ImGui::TableNextColumn();
                ImGui::Text("%.0f%%", totalMs / trackTotalMs * 100.0f);

                if (hasChildren && opened)
                    ImGui::TreePop();
                ImGui::Unindent(static_cast<float>(entry.Depth) * ImGui::GetTreeNodeToLabelSpacing());
                ImGui::PopID();
            }
        }

        ImGui::EndTable();
    }
}
