#include "PropertyControls/ColorPropertyControl.h"
#include <imgui/imgui.h>
#include <Pulsar/EngineMath.h>

namespace pulsared
{
    bool ColorPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop, std::span<Attribute*> attrs)
    {
        assert(prop && prop->GetType() == GetPropertyType());

        auto f = static_cast<jxcorlib::math::BoxingColor4f*>(prop);
        ImGui::PushItemWidth(-1);
        float color[4] = { pulsar::math::LinearToSRGB(f->r), pulsar::math::LinearToSRGB(f->g), pulsar::math::LinearToSRGB(f->b), f->a };
        bool changed = ImGui::ColorEdit4(("##" + name).c_str(), color);
        if (changed)
        {
            f->r = pulsar::math::SRGBToLinear(color[0]);
            f->g = pulsar::math::SRGBToLinear(color[1]);
            f->b = pulsar::math::SRGBToLinear(color[2]);
            f->a = color[3];
        }

        ImGui::PopItemWidth();
        return changed;
    }
}