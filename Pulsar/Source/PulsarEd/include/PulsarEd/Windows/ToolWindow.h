#pragma once
#include "EditorWindow.h"

namespace pulsared
{
    class ToolWindow : public EditorWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ToolWindow, EditorWindow);
    public:
        ToolWindow();
        template<typename T>
        static void OpenToolWindow()
        {
            mksptr(new T)->Open();
        }

        virtual string_view GetWindowDisplayName() const override { return ICON_FK_TELEVISION "Tool"; }
        virtual ImGuiWindowFlags GetGuiWindowFlags() const {
            return ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoCollapse;
        }
        virtual string GetWindowName() const override;
        virtual void DrawImGui() override;

    };
}