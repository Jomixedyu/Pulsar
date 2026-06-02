#include "Mask8PropertyControl.h"
#include <imgui/imgui.h>

namespace pulsared
{
    bool Mask8PropertyControl::OnDrawImGui(const jxcorlib::string& name, jxcorlib::Type* type, jxcorlib::Object* prop, std::span<Attribute*> attrs)
    {
        assert(prop && prop->GetType() == GetPropertyType());

        auto f = static_cast<BoxingMask8*>(prop);
        ImGui::PushItemWidth(-1);

        auto mask = (int)f->GetValue();
        bool changed = ImGui::DragInt(("##" + name).c_str(), &mask, 1, 0, 255);
        if (changed)
        {
            f->SetValue((uint8_t)mask);
        }
        ImGui::PopItemWidth();
        return changed;
    }

}
