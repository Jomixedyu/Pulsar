#include "PropertyControls/EnumPropertyControl.h"
#include <imgui/imgui.h>
#include <ranges>

namespace pulsared
{
    bool EnumPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop)
    {
        assert(prop && prop->GetType()->IsSubclassOf(GetPropertyType()));

        auto b = static_cast<Enum*>(prop);
        ImGui::PushItemWidth(-1);
        auto value = b->get_unboxing_value();

        auto enumDefines = b->GetType()->GetEnumDefinitions();

        string curEnumName;
        for (auto& v : *enumDefines)
        {
            if (v.second == value)
            {
                curEnumName = v.first;
                break;
            }
        }

        auto selectedIndex = value;
        if (ImGui::BeginCombo(name.c_str(), curEnumName.c_str()))
        {
            for (int i = 0; auto& v : *enumDefines)
            {
                if (ImGui::Selectable(v.first.c_str(), value == i))
                {
                    selectedIndex = i;
                }
                ++i;
            }
            ImGui::EndCombo();
        }

        bool changed = selectedIndex != value;

        if (changed)
        {
            b->SetValue(selectedIndex);
        }

        ImGui::PopItemWidth();
        return changed;
    }

} // namespace pulsared