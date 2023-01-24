#include "ColorPropertyControl.h"
#include <ThirdParty/imgui/imgui.h>

namespace apatiteed
{
    void ColorPropertyControl::OnDrawImGui(const string& name, sptr<Object> prop)
    {
        assert(prop && prop->GetType() == GetPropertyType());
        Type* type = prop->GetType();

        sptr<jxcorlib::math::BoxingLinearColorf> f = sptr_cast<jxcorlib::math::BoxingLinearColorf>(prop);
        ImGui::PushItemWidth(-1);
        float color[4] = { f->r, f->g, f->b, f->a };
        ImGui::ColorPicker4(("##" + name).c_str(), color);
        f->r = color[0];
        f->g = color[1];
        f->b = color[2];
        f->a = color[3];
        ImGui::PopItemWidth();
    }
}