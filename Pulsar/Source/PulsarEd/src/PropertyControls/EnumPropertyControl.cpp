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
        auto enumValue = b->get_unboxing_value();

        auto enumAccessor =  b->GetType()->GetEnumAccessors();

        string curEnumName;
        enumAccessor->GetName(enumValue, &curEnumName);


        auto selectedValue = enumValue;
        if (ImGui::BeginCombo(name.c_str(), curEnumName.c_str()))
        {
            for (auto& v : enumAccessor->i2s)
            {
                if (ImGui::Selectable(v.second.c_str(), enumValue == v.first))
                {
                    selectedValue = v.first;
                }
            }
            ImGui::EndCombo();
        }

        bool changed = selectedValue != enumValue;

        if (changed)
        {
            b->SetValue(selectedValue);
        }

        ImGui::PopItemWidth();
        return changed;
    }

} // namespace pulsared