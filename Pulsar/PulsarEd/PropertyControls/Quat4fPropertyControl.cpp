#include "Quat4fPropertyControl.h"
#include <ThirdParty/imgui/imgui.h>

namespace pulsared
{
	void Quat4fPropertyControl::OnDrawImGui(const string& name, sptr<Object> prop)
	{
        assert(prop && prop->GetType() == GetPropertyType());
        Type* type = prop->GetType();
        auto q = sptr_cast<pulsar::math::BoxingQuat4f>(prop);
        ImGui::PushItemWidth(-1);

        float f[] = { q->x, q->y, q->z, q->w };
        ImGui::DragFloat4(("##" + name).c_str(), f, 0.2f, 0.f, 0.f, "%.3f");

        q->x = f[0];
        q->y = f[1];
        q->z = f[2];
        q->w = f[3];

        ImGui::PopItemWidth();
	}
}