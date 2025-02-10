#pragma once
#include "Editors/EditorWindow.h"

namespace pulsared
{
    class SceneEditorWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::SceneEditorWindow, EditorWindow);
    public:
        using base::base;

    protected:
        void OnOpen() override;
        void OnClose() override;
        void OnDrawImGui(float dt) override;

    };
}