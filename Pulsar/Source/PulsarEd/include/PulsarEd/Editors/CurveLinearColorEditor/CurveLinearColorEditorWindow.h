#pragma once
#include "Pulsar/Assets/CurveLinearColor.h"
#include "PulsarEd/Editors/CurveEditor/CurveEditorWindow.h"

namespace pulsared
{
    class CurveLinearColorEditorWindow : public CurveEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::CurveLinearColorEditorWindow, CurveEditorWindow);
        DEFINE_ASSET_EDITOR(CurveLinearColor, true);

    public:
        void OnOpen() override;
        void OnClose() override;

        void OnDrawImGui(float dt) override;

    private:
        RCPtr<CurveLinearColor> m_colorCurve;
    };
} // namespace pulsared