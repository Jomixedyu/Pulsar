#pragma once
#include "EdGuiWindow.h"

namespace pulsared
{
    class StatusBarWindow : public EdGuiWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::StatusBarWindow, EdGuiWindow);
    public:
        //virtual ImGuiWindowFlags GetGuiWindowFlags() const override {
        //    return ImGuiWindowFlags_None | ImGuiWindowFlags_::ImGuiWindowFlags_MenuBar;
        //}
        StatusBarWindow() {}
        virtual void OnOpen() override;
        virtual void OnClose() override;
        static string_view StaticWindowName() { return "StatusBar"; }
        virtual string_view GetWindowDisplayName() const override { return StaticWindowName(); }
        virtual void OnDrawImGui(float dt) override;
        virtual void DrawImGui(float dt) override;

    private:
        virtual void OnWindowResize();
    private:

    };
}