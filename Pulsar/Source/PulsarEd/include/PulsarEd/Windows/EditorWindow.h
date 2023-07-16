#pragma once
#include <pulsared/Assembly.h>
#include <ThirdParty/imgui/imgui.h>
#include "EditorUI.h"

namespace pulsared
{
    class EditorWindow : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_PulsarEd, pulsared::EditorWindow, Object);

    public:
        virtual string_view GetWindowName() const;
        virtual void DrawImGui();
        virtual ImGuiWindowFlags GetGuiWindowFlags() const {
            return ImGuiWindowFlags_None;
        }

    public:
        bool get_is_opened() const { return is_opened; }
        int32_t get_window_id() const { return this->window_id_; }
        EditorWindow();
    public:
        bool Open();
        void Close();
    protected:
        virtual void OnDrawImGui();
        virtual void OnOpen() {}
        virtual void OnClose() {}

    protected:
        bool is_opened = false;
    private:
        int32_t window_id_;
    };
    CORELIB_DECL_SHORTSPTR(EditorWindow);
}