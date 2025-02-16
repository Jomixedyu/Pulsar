#include "PulsarEd/Editors/CurveLinearColorEditor/CurveLinearColorEditorWindow.h"

#include "ImCurveEdit.h"
#include "ImGuiExt.h"
#include "PropertyControls/PropertyControl.h"

namespace pulsared
{

    void CurveLinearColorEditorWindow::OnOpen()
    {
        base::OnOpen();
        m_colorCurve = cref_cast<CurveLinearColor>(m_assetObject);
    }
    void CurveLinearColorEditorWindow::OnClose()
    {
        base::OnClose();
    }

    struct RampEdit : ImCurveEdit::Delegate
    {
        CurveLinearColor* m_curve;
        array_list<array_list<ImVec2>> m_imData;
        ImVec2 m_min{};
        ImVec2 m_max{};

        RampEdit(CurveLinearColor* curve) : m_curve(curve)
        {
            BuildImData();
        }
        size_t GetCurveCount() override { return m_curve->GetCurveCount(); }
        ImVec2& GetMin() override { return m_min; }
        ImVec2& GetMax() override { return m_max; }
        size_t GetPointCount(size_t curveIndex) override
        {
            return m_curve->GetCurveData(curveIndex)->GetKeyCount();
        }
        uint32_t GetCurveColor(size_t curveIndex) override
        {
            return 0xFFFFFFFF;
        }

        void BuildImData()
        {
            m_imData.resize(m_curve->GetCurveCount());
            for (int curveIndex = 0; curveIndex < m_imData.size(); ++curveIndex)
            {
                auto& curve = m_curve->GetCurveData(curveIndex);
                m_imData[curveIndex].resize(curve->GetKeyCount());
                auto keyCount = curve->GetKeyCount();
                for (int keyIndex = 0; keyIndex < keyCount; ++keyIndex)
                {
                    m_imData[curveIndex][keyIndex] = ImVec2( curve->GetKey(keyIndex).Time, curve->GetKey(keyIndex).Value );
                }
            }
        }

        ImVec2* GetPoints(size_t curveIndex) override
        {
            return m_imData[curveIndex].data();
        }
        int EditPoint(size_t curveIndex, int pointIndex, ImVec2 value) override
        {
            auto key = m_curve->GetCurveData(curveIndex)->GetKey(pointIndex);
            key.Time = value.x;
            key.Value = value.y;
            m_curve->GetCurveData(curveIndex)->SetKey(pointIndex, key);
            BuildImData();
            return 0;
        }
        void AddPoint(size_t curveIndex, ImVec2 value) override
        {
            auto key = CurveKey{};
            key.Time = value.x;
            key.Value = value.y;
            m_curve->GetCurveData(curveIndex)->AddKey(key);

            BuildImData();
        }
    };

    void CurveLinearColorEditorWindow::OnDrawImGui(float dt)
    {
        base::OnDrawImGui(dt);
        if (!m_colorCurve)
        {
            return;
        }

        auto edit = RampEdit { m_colorCurve.GetPtr() };

        auto id = ImGui::GetID("CurveEdit");
        // ImCurveEdit::Edit(edit, ImVec2{500, 500}, id);

        if (PImGui::PropertyGroup("Curve Color"))
        {
            auto type = cltypeof<CurveLinearColor>();
            PImGui::ObjectFieldProperties(type, type, m_colorCurve.GetPtr(), m_colorCurve.GetPtr(), false);
        }

        // ramp
        // curve editor
    }
} // namespace pulsared