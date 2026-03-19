#include "PulsarEd/Editors/CurveLinearColorEditor/CurveLinearColorEditorWindow.h"

#include "ImGuiExt.h"
#include "ImGradientHDR.h"
#include "PropertyControls/PropertyControl.h"
#include "imgui/imgui_internal.h"

namespace pulsared
{

    void CurveLinearColorEditorWindow::OnOpen()
    {
        base::OnOpen();
        m_colorCurve = cast<CurveLinearColor>(m_assetObject);
    }
    void CurveLinearColorEditorWindow::OnClose()
    {
        base::OnClose();
    }

    void CurveLinearColorEditorWindow::OnDrawImGui(float dt)
    {
        base::OnDrawImGui(dt);
        if (!m_colorCurve)
        {
            return;
        }

        auto id = ImGui::GetID("cx");
        ImGradientHDRState state;

        auto keyCount = m_colorCurve->GetCurveData(0)->GetKeyCount();
        auto curveCount = m_colorCurve->GetCurveCount();

        for (int i = 0; i < keyCount; ++i)
        {
            auto time = m_colorCurve->GetCurveData(0)->GetKey(i).Time;
            std::array<float, 3> color{};
            color[0] = m_colorCurve->GetCurveData(0)->GetKey(i).Value;
            color[1] = m_colorCurve->GetCurveData(1)->GetKey(i).Value;
            color[2] = m_colorCurve->GetCurveData(2)->GetKey(i).Value;
            state.AddColorMarker(time, color, 1);;
        }

        auto oldKeyCount = state.ColorCount;

        if (ImGradientHDR(id,state, temporaryState))
        {
            auto newKeyCount = state.ColorCount;
            for (int i = oldKeyCount; i < newKeyCount; ++i)
            {
                m_colorCurve->InsertColorKey();
            }

            for (int keyIndex = 0; keyIndex < state.ColorCount; ++keyIndex)
            {
                auto keys = m_colorCurve->GetColorsKey(keyIndex);
                for (int colorChannelIndex = 0; colorChannelIndex < 3; ++colorChannelIndex)
                {
                    keys[colorChannelIndex].Time = state.Colors[keyIndex].Position;
                    keys[colorChannelIndex].Value = state.Colors[keyIndex].Color[colorChannelIndex];
                }
                m_colorCurve->SetColorsKey(keyIndex, keys);

            }
            m_colorCurve->PostEditChange(nullptr);
        }

        if (temporaryState.selectedIndex >= 0)
        {
            auto color = m_colorCurve->GetColorsKey(temporaryState.selectedIndex);
            float color4[4];
            color4[0] = color[0].Value;
            color4[1] = color[1].Value;
            color4[2] = color[2].Value;
            color4[3] = color[3].Value;

            if (ImGui::ColorEdit4("color", color4))
            {
                color[0].Value = color4[0];
                color[1].Value = color4[1];
                color[2].Value = color4[2];
                color[3].Value = color4[3];

                m_colorCurve->SetColorsKey(temporaryState.selectedIndex, color);
                m_colorCurve->PostEditChange(nullptr);
            }
        }


        if (PImGui::PropertyGroup("Curve Color"))
        {
            auto type = cltypeof<CurveLinearColor>();
            PImGui::ObjectFieldProperties(type, type, m_colorCurve.GetPtr(), m_colorCurve.GetPtr(), false);
        }


    }
} // namespace pulsared