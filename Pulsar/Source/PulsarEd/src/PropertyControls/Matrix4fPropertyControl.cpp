#include "PropertyControls/Matrix4fPropertyControl.h"
#include <ThirdParty/imgui/imgui.h>

namespace pulsared
{
    void Matrix4fPropertyControl::OnDrawImGui(const string& name, sptr<Object> prop)
    {
        assert(prop && prop->GetType() == GetPropertyType());
        Type* type = prop->GetType();
        auto mat = sptr_cast<pulsar::math::BoxingMatrix4f>(prop);
        ImGui::PushItemWidth(-1);

        float a[] = { mat->v1x, mat->v2x, mat->v3x, mat->v4x };
        float b[] = { mat->v1y, mat->v2y, mat->v3y, mat->v4y };
        float c[] = { mat->v1z, mat->v2z, mat->v3z, mat->v4z };
        float d[] = { mat->v1w, mat->v2w, mat->v3w, mat->v4w };

        ImGui::DragFloat4(("##a" + name).c_str(), a, 0.2f, 0.0f, 0.0f, "%0.3f");
        ImGui::DragFloat4(("##b" + name).c_str(), b, 0.2f, 0.0f, 0.0f, "%0.3f");
        ImGui::DragFloat4(("##c" + name).c_str(), c, 0.2f, 0.0f, 0.0f, "%0.3f");
        ImGui::DragFloat4(("##d" + name).c_str(), d, 0.2f, 0.0f, 0.0f, "%0.3f");

        mat->v1x = a[0]; mat->v2x = a[1]; mat->v3x = a[2]; mat->v4x = a[3];
        mat->v1y = b[0]; mat->v2y = b[1]; mat->v3y = b[2]; mat->v4y = b[3];
        mat->v1z = c[0]; mat->v2z = c[1]; mat->v3z = c[2]; mat->v4z = c[3];
        mat->v1w = d[0]; mat->v2w = d[1]; mat->v3w = d[2]; mat->v4w = d[3];

        ImGui::PopItemWidth();
    }
}