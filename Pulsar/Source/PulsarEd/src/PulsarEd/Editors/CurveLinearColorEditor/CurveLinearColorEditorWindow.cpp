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

        // Load RGB color markers
        auto keyCount = m_colorCurve->GetKeyCount();
        for (int i = 0; i < keyCount; ++i)
        {
            auto time = m_colorCurve->GetKeyR(i).Time;
            std::array<float, 3> color{};
            color[0] = m_colorCurve->GetKeyR(i).Value;
            color[1] = m_colorCurve->GetKeyG(i).Value;
            color[2] = m_colorCurve->GetKeyB(i).Value;
            state.AddColorMarker(time, color, 1);
        }

        // Load alpha markers
        auto alphaKeyCount = m_colorCurve->GetKeyCountA();
        for (int i = 0; i < alphaKeyCount; ++i)
        {
            auto time = m_colorCurve->GetKeyA(i).Time;
            state.AddAlphaMarker(time, m_colorCurve->GetKeyA(i).Value);
        }

        bool changed = ImGradientHDR(id, state, temporaryState);

        // Delete selected marker
        if (temporaryState.selectedIndex >= 0 && ImGui::IsKeyPressed(ImGuiKey_Delete))
        {
            if (temporaryState.selectedMarkerType == ImGradientHDRMarkerType::Color)
            {
                state.RemoveColorMarker(temporaryState.selectedIndex);
            }
            else if (temporaryState.selectedMarkerType == ImGradientHDRMarkerType::Alpha)
            {
                state.RemoveAlphaMarker(temporaryState.selectedIndex);
            }
            temporaryState.selectedIndex = -1;
            temporaryState.selectedMarkerType = ImGradientHDRMarkerType::Unknown;
            changed = true;
        }

        if (changed)
        {
            m_colorCurve->ClearKeys();

            for (int i = 0; i < state.ColorCount; ++i)
            {
                CurveKey r{}, g{}, b{};
                r.Time = g.Time = b.Time = state.Colors[i].Position;
                r.Value = state.Colors[i].Color[0];
                g.Value = state.Colors[i].Color[1];
                b.Value = state.Colors[i].Color[2];
                m_colorCurve->AddKeyR(r);
                m_colorCurve->AddKeyG(g);
                m_colorCurve->AddKeyB(b);
            }

            for (int i = 0; i < state.AlphaCount; ++i)
            {
                CurveKey a{};
                a.Time = state.Alphas[i].Position;
                a.Value = state.Alphas[i].Alpha;
                m_colorCurve->AddKeyA(a);
            }

            m_colorCurve->PostEditChange(nullptr);
        }

        if (temporaryState.selectedIndex >= 0)
        {
            if (temporaryState.selectedMarkerType == ImGradientHDRMarkerType::Color)
            {
                auto r = m_colorCurve->GetKeyR(temporaryState.selectedIndex);
                auto g = m_colorCurve->GetKeyG(temporaryState.selectedIndex);
                auto b = m_colorCurve->GetKeyB(temporaryState.selectedIndex);
                float color3[3] = { r.Value, g.Value, b.Value };

                if (ImGui::ColorEdit3("color", color3))
                {
                    r.Value = color3[0];
                    g.Value = color3[1];
                    b.Value = color3[2];
                    m_colorCurve->SetKeyR(temporaryState.selectedIndex, r);
                    m_colorCurve->SetKeyG(temporaryState.selectedIndex, g);
                    m_colorCurve->SetKeyB(temporaryState.selectedIndex, b);
                    m_colorCurve->PostEditChange(nullptr);
                }
            }
            else if (temporaryState.selectedMarkerType == ImGradientHDRMarkerType::Alpha)
            {
                auto a = m_colorCurve->GetKeyA(temporaryState.selectedIndex);
                float alpha = a.Value;
                if (ImGui::SliderFloat("alpha", &alpha, 0.0f, 1.0f))
                {
                    a.Value = alpha;
                    m_colorCurve->SetKeyA(temporaryState.selectedIndex, a);
                    m_colorCurve->PostEditChange(nullptr);
                }
            }
        }


        if (PImGui::PropertyGroup("Curve Color"))
        {
            auto type = cltypeof<CurveLinearColor>();
            PImGui::ObjectFieldProperties(type, type, m_colorCurve.GetPtr(), m_colorCurve.GetPtr(), false);
        }


    }
} // namespace pulsared