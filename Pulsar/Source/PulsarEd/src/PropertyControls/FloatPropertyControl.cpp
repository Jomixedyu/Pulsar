#include "PropertyControls/FloatPropertyControl.h"
#include <ThirdParty/imgui/imgui.h>

namespace pulsared
{
    void FloatPropertyControl::OnDrawImGui(const string& name, sptr<Object> prop)
    {
        assert(prop && prop->GetType() == GetPropertyType());
        Type* type = prop->GetType();
        sptr<Single32> f = sptr_cast<Single32>(prop);
        ImGui::PushItemWidth(-1);
        ImGui::DragFloat(("##" + name).c_str(), &f->value, 0.2f);
        ImGui::PopItemWidth();
    }
}