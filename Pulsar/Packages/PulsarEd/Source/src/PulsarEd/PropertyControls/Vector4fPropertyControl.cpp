#include "PropertyControls/Vector4fPropertyControl.h"
#include <Pulsar/Meta/PropertyStyleAttributes.h>
#include <imgui/imgui.h>
#include <Pulsar/EngineMath.h>

namespace pulsared
{
    bool Vector4fPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop, std::span<Attribute*> attrs)
    {
        assert(prop && prop->GetType() == GetPropertyType());
        auto f = static_cast<pulsar::math::BoxingVector4f*>(prop);
        ImGui::PushItemWidth(-1);

        bool isColor = false;
        for (auto* attr : attrs)
        {
            if (attr->GetType() == cltypeof<pulsar::ColorEditAttribute>() ||
                attr->GetType()->IsSubclassOf(cltypeof<pulsar::ColorEditAttribute>()))
            {
                isColor = true;
                break;
            }
        }

        bool changed = false;
        if (isColor)
        {
            float color[4] = { pulsar::math::LinearToSRGB(f->x), pulsar::math::LinearToSRGB(f->y), pulsar::math::LinearToSRGB(f->z), f->w };
            auto popupId = "ColorPopup_" + name;
            if (ImGui::ColorButton(("##btn_" + name).c_str(), ImVec4{color[0], color[1], color[2], color[3]}, 0, ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetFrameHeight())))
            {
                ImGui::OpenPopup(popupId.c_str());
            }
            if (ImGui::BeginPopup(popupId.c_str()))
            {
                if (ImGui::ColorPicker4(("##picker_" + name).c_str(), color,
                    ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_NoSmallPreview))
                {
                    changed = true;
                }
                ImGui::EndPopup();
            }
            if (changed)
            {
                f->x = pulsar::math::SRGBToLinear(color[0]);
                f->y = pulsar::math::SRGBToLinear(color[1]);
                f->z = pulsar::math::SRGBToLinear(color[2]);
                f->w = color[3];
            }
        }
        else
        {
            float f4[] = { f->x, f->y, f->z, f->w };
            changed = ImGui::DragFloat4(("##" + name).c_str(), f4, 0.002f, 0.f, 0.f, "%.3f");
            if (changed)
            {
                f->x = f4[0];
                f->y = f4[1];
                f->z = f4[2];
                f->w = f4[3];
            }
        }
        ImGui::PopItemWidth();
        return changed;
    }
}
