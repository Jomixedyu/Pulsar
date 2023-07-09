#include "PropertyControls/Vector2fPropertyControl.h"
#include <ThirdParty/imgui/imgui.h>

namespace pulsared
{
	void Vector2fPropertyControl::OnDrawImGui(const string& name, sptr<Object> prop)
	{
        assert(prop && prop->GetType() == GetPropertyType());
        Type* type = prop->GetType();
        auto f = sptr_cast<pulsar::math::BoxingVector2f>(prop);
        ImGui::PushItemWidth(-1);
        float f3[] = { f->x, f->y};
        ImGui::DragFloat2(("##" + name).c_str(), f3, 0.2f);
        f->x = f3[0];
        f->y = f3[1];
        ImGui::PopItemWidth();
	}
}