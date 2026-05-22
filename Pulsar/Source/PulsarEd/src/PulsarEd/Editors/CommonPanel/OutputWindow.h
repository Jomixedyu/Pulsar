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

        const ViewportFrame* GetViewportFrame() const { return m_viewportFrame; }

        // Route global input events to the World currently displayed by this window.
        // Returns the World that received input (may be nullptr).
        World* RouteInput(const std::vector<uinput::InputEvent>& events);

    private:
        void OnOpenWorkspace();
        void OnCloseWorkspace();

        ViewportFrame* m_viewportFrame = nullptr;
    };
}
