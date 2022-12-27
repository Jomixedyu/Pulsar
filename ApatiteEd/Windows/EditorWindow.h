#pragma once
#include <apatiteed/Assembly.h>
#include <ThirdParty/imgui/imgui.h>

namespace apatiteed
{
    class EditorWindow
    {
    public:
        virtual string_view GetWindowName() const;
        virtual void DrawImGui();
        virtual ImGuiWindowFlags GetGuiWindowFlags() const {
            return ImGuiWindowFlags_None;
        }

    public:
        bool get_is_opened() const { return is_opened; }
        void Open();
        void Close();
        EditorWindow();
    protected:
        virtual void OnDrawImGui();
        virtual void OnOpen() {}
        virtual void OnClose() {}

    protected:
        bool is_opened = false;
    };
}