#include "PropertyControls/ColorPropertyControl.h"
#include <imgui/imgui.h>

namespace pulsared
{
    bool ColorPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop)
    {
        assert(prop && prop->GetType() == GetPropertyType());

        auto f = static_cast<jxcorlib::math::BoxingColor4f*>(prop);
        ImGui::PushItemWidth(-1);
        float color[4] = { f->r, f->g, f->b, f->a };
        bool changed = ImGui::ColorEdit4(("##" + name).c_str(), color);
        //bool changed = ImGui::ColorButton(("##" + name).c_str(), ImVec4{f->r, f->g, f->b, f->a});
        if (changed)
        {
            f->r = color[0];
            f->g = color[1];
            f->b = color[2];
            f->a = color[3];
        }

        ImGui::PopItemWidth();
        return changed;
    }
}