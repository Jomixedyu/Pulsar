#pragma once
#include "Windows/PanelWindow.h"

namespace pulsared
{
    class ViewportFrame;

    class OutputWindow : public PanelWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::OutputWindow, PanelWindow);
    public:
        virtual ImGuiWindowFlags GetGuiWindowFlags() const override {
            return ImGuiWindowFlags_None | ImGuiWindowFlags_MenuBar;
        }
        static string_view StaticWindowName() { return ICON_FK_TELEVISION " Output" "###Output"; }
        virtual string_view GetWindowDisplayName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui(float dt) override;

        virtual void OnOpen() override;
        virtual void OnClose() override;

    private:
        void OnOpenWorkspace();
        void OnCloseWorkspace();

        ViewportFrame* m_viewportFrame = nullptr;
    };
}
