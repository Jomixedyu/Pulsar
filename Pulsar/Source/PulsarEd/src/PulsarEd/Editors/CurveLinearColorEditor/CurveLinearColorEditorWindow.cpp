#include "PulsarEd/Editors/CurveLinearColorEditor/CurveLinearColorEditorWindow.h"

#include "ImGuiExt.h"
#include "ImGradientHDR.h"
#include "PropertyControls/PropertyControl.h"
#include "imgui/imgui_internal.h"

#include <CoreLib.Platform/Window.h>
#include <CoreLib.Serialization/json.hpp>
#include <fstream>

namespace pulsared
{
    using pulsar::math::LinearToSRGB;
    using pulsar::math::SRGBToLinear;

    void CurveLinearColorEditorWindow::OnOpen()
    {
        base::OnOpen();
        m_colorCurve = cast<CurveLinearColor>(m_assetObject);
    }
    void CurveLinearColorEditorWindow::OnClose()
    {
        base::OnClose();
    }

    ImGuiWindowClass CurveLinearColorEditorWindow::GetGuiWindowClass() const
    {
        ImGuiWindowClass winClass{};
        winClass.DockingAllowUnclassed = false;
        winClass.ClassId = ImGui::GetID((std::to_string(GetWindowId()) + "CurveLinearColorEditor").c_str());
        return winClass;
    }

    void CurveLinearColorEditorWindow::ImportFromUEJson()
    {
        std::filesystem::path selectedPath;
        if (!platform::window::OpenFileDialog(platform::window::GetMainWindowHandle(), "UE Curve JSON(*.json)|*.json;", "", &selectedPath))
        {
            return;
        }

        std::ifstream file(selectedPath);
        if (!file.is_open())
        {
            return;
        }

        nlohmann::json json;
        file >> json;

        if (!json.contains("floatCurves") || !json["floatCurves"].is_array())
        {
            return;
        }

        auto& curves = json["floatCurves"];
        if (curves.size() < 4)
        {
            return;
        }

        m_colorCurve->ClearKeys();

        auto ParseChannel = [&](int channelIndex, auto addKey)
        {
            auto& curve = curves[channelIndex];
            if (!curve.contains("keys") || !curve["keys"].is_array())
                return;

            for (auto& keyJson : curve["keys"])
            {
                CurveKey key{};
                key.Time = keyJson.value("time", 0.0f);
                key.Value = keyJson.value("value", 0.0f);

                auto interpMode = keyJson.value("interpMode", "RCIM_Linear");
                if (interpMode == "RCIM_Linear")
                    key.InterpMode = CurveInterpMode::Linear;
                else if (interpMode == "RCIM_Constant")
                    key.InterpMode = CurveInterpMode::Constant;
                else if (interpMode == "RCIM_Cubic")
                    key.InterpMode = CurveInterpMode::Cubic;
                else
                    key.InterpMode = CurveInterpMode::Linear;

                addKey(key);
            }
        };

        ParseChannel(0, [this](const CurveKey& key) { m_colorCurve->AddKeyR(key); });
        ParseChannel(1, [this](const CurveKey& key) { m_colorCurve->AddKeyG(key); });
        ParseChannel(2, [this](const CurveKey& key) { m_colorCurve->AddKeyB(key); });
        ParseChannel(3, [this](const CurveKey& key) { m_colorCurve->AddKeyA(key); });

        m_colorCurve->PostEditChange(nullptr);
        AssetDatabase::MarkDirty(m_assetObject);
    }

    void CurveLinearColorEditorWindow::OnDrawImGui(float dt)
    {
        base::OnDrawImGui(dt);
        if (!m_colorCurve)
        {
            return;
        }

        if (ImGui::Button("Import from UE JSON"))
        {
            ImportFromUEJson();
        }

        ImGui::PushID(GetWindowId());
        auto id = ImGui::GetID("cx");
        ImGradientHDRState state;

        // Load RGB color markers (Linear -> sRGB for display)
        auto keyCount = m_colorCurve->GetKeyCount();
        for (int i = 0; i < keyCount; ++i)
        {
            auto time = m_colorCurve->GetKeyR(i).Time;
            std::array<float, 3> color{};
            color[0] = LinearToSRGB(m_colorCurve->GetKeyR(i).Value);
            color[1] = LinearToSRGB(m_colorCurve->GetKeyG(i).Value);
            color[2] = LinearToSRGB(m_colorCurve->GetKeyB(i).Value);
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
                r.Value = SRGBToLinear(state.Colors[i].Color[0]);
                g.Value = SRGBToLinear(state.Colors[i].Color[1]);
                b.Value = SRGBToLinear(state.Colors[i].Color[2]);
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
            AssetDatabase::MarkDirty(m_assetObject);
        }

        if (temporaryState.selectedIndex >= 0)
        {
            if (temporaryState.selectedMarkerType == ImGradientHDRMarkerType::Color)
            {
                auto r = m_colorCurve->GetKeyR(temporaryState.selectedIndex);
                auto g = m_colorCurve->GetKeyG(temporaryState.selectedIndex);
                auto b = m_colorCurve->GetKeyB(temporaryState.selectedIndex);

                float time = r.Time;
                if (ImGui::DragFloat("Time", &time, 0.01f, 0.0f, 1.0f))
                {
                    r.Time = g.Time = b.Time = time;
                    m_colorCurve->SetKeyR(temporaryState.selectedIndex, r);
                    m_colorCurve->SetKeyG(temporaryState.selectedIndex, g);
                    m_colorCurve->SetKeyB(temporaryState.selectedIndex, b);
                    m_colorCurve->PostEditChange(nullptr);
                    AssetDatabase::MarkDirty(m_assetObject);
                }

                float color3[3] = { LinearToSRGB(r.Value), LinearToSRGB(g.Value), LinearToSRGB(b.Value) };
                if (ImGui::ColorEdit3("color", color3, ImGuiColorEditFlags_Float | ImGuiColorEditFlags_HDR))
                {
                    r.Value = SRGBToLinear(color3[0]);
                    g.Value = SRGBToLinear(color3[1]);
                    b.Value = SRGBToLinear(color3[2]);
                    m_colorCurve->SetKeyR(temporaryState.selectedIndex, r);
                    m_colorCurve->SetKeyG(temporaryState.selectedIndex, g);
                    m_colorCurve->SetKeyB(temporaryState.selectedIndex, b);
                    m_colorCurve->PostEditChange(nullptr);
                    AssetDatabase::MarkDirty(m_assetObject);
                }
            }
            else if (temporaryState.selectedMarkerType == ImGradientHDRMarkerType::Alpha)
            {
                auto a = m_colorCurve->GetKeyA(temporaryState.selectedIndex);

                float time = a.Time;
                if (ImGui::DragFloat("Time", &time, 0.01f, 0.0f, 1.0f))
                {
                    a.Time = time;
                    m_colorCurve->SetKeyA(temporaryState.selectedIndex, a);
                    m_colorCurve->PostEditChange(nullptr);
                    AssetDatabase::MarkDirty(m_assetObject);
                }

                float alpha = a.Value;
                if (ImGui::SliderFloat("alpha", &alpha, 0.0f, 1.0f))
                {
                    a.Value = alpha;
                    m_colorCurve->SetKeyA(temporaryState.selectedIndex, a);
                    m_colorCurve->PostEditChange(nullptr);
                    AssetDatabase::MarkDirty(m_assetObject);
                }
            }
        }

        ImGui::PopID();

        if (PImGui::PropertyGroup("Curve Color"))
        {
            auto type = cltypeof<CurveLinearColor>();
            PImGui::ObjectFieldProperties(type, type, m_colorCurve.GetPtr(), m_colorCurve.GetPtr(), false);
        }


    }
} // namespace pulsared