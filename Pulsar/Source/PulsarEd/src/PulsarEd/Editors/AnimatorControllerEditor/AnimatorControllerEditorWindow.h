#pragma once
#include "Editors/AssetEditor/AssetEditorWindow.h"
#include <Pulsar/Assets/AnimatorController.h>

namespace pulsared
{
    class AnimatorControllerEditorWindow : public AssetEditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::AnimatorControllerEditorWindow, AssetEditorWindow);
        DEFINE_ASSET_EDITOR(AnimatorController, false);
    public:
        string_view GetWindowDisplayName() const override { return "Animator Controller"; }
    protected:
        void OnOpen() override;
        void OnDrawAssetEditor(float dt) override;
    private:
        void DrawParamsPanel();
        void DrawStatesPanel();
        void DrawTransitionsPanel();

        RCPtr<pulsar::AnimatorController> m_controller;

        // 编辑状态
        char m_addParamName[64]  = {};
        char m_addStateName[64]  = {};
        int  m_addParamType      = 0;   // 0=Bool,1=Int,2=Float,3=Trigger
        int  m_selectedStateIdx  = -1;
        int  m_selectedTransIdx  = -1;
        int  m_addCondParamIdx   = 0;
        int  m_addTransFrom      = 0;
        int  m_addTransTo        = 0;
        bool m_addTransAnyState  = false;
    };
}
