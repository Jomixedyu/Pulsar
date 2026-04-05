#include "PropertyControls/PathPropertyControl.h"
#include <imgui/imgui.h>
#include "CoreLib.Platform/Window.h"

namespace pulsared
{
    static char s_pathBuf[2048];

    bool PathPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop)
    {
        assert(prop && prop->GetType() == GetPropertyType());

        auto* value = static_cast<jxcorlib::Path*>(prop);
        bool changed = false;

        // Fill buffer with current path string
        const string current = value->ToString();
        StringUtil::strcpy(s_pathBuf, current);

        ImGui::PushID(name.c_str());

        // Input box takes up remaining width minus the browse button
        const float btnWidth = 24.f;
        const float spacing  = ImGui::GetStyle().ItemSpacing.x;
        ImGui::PushItemWidth(-btnWidth - spacing);

        if (ImGui::InputText("##path", s_pathBuf, sizeof(s_pathBuf)))
        {
            value->SetPath(jxcorlib::path(s_pathBuf));
            changed = true;
        }

        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Browse button "..."
        if (ImGui::Button("...", ImVec2(btnWidth, 0)))
        {
            using namespace jxcorlib::platform;
            std::filesystem::path picked;
            if (window::OpenFileDialog(window::GetMainWindowHandle(), "All Files(*.*)|*.*;", "", &picked))
            {
                value->SetPath(picked);
                changed = true;
            }
        }
        if (ImGui::IsItemHovered())
            ImGui::SetTooltip("Browse...");

        ImGui::PopID();
        return changed;
    }
}
