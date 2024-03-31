#include "ImGuiExt.h"

namespace ImGuiExt
{

    bool GetMousePosOnContentRegion(ImVec2& vec)
    {
        ImVec2 vMin = ImGui::GetWindowContentRegionMin();
        ImVec2 vMax = ImGui::GetWindowContentRegionMax();
        vMin.x += ImGui::GetWindowPos().x;
        vMin.y += ImGui::GetWindowPos().y;
        auto mpos = ImGui::GetMousePos();
        vec = ImVec2{mpos.x - vMin.x, mpos.y - vMin.y};
        return !(vec.x < 0 || vec.y < 0 || vec.x > vMax.x || vec.y > vMax.y);
    }

    ImVec2 GetContentSize()
    {
        ImVec2 vMin = ImGui::GetWindowContentRegionMin();
        ImVec2 vMax = ImGui::GetWindowContentRegionMax();
        vMin.x += ImGui::GetWindowPos().x;
        vMin.y += ImGui::GetWindowPos().y;
        vMax.x += ImGui::GetWindowPos().x;
        vMax.y += ImGui::GetWindowPos().y;
        return {vMax.x - vMin.x, vMax.y - vMin.y};
    }

} // namespace ImGuiExt