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
        bool GetIsOpened() const { return m_isOpened; }
        int32_t GetWindowId() const { return this->m_windowId; }
        EditorWindow();
    public:
        bool Open();
        void Close();
    protected:
        virtual void OnDrawImGui();
        virtual void OnOpen() {}
        virtual void OnClose() {}

    protected:
        bool m_isOpened = false;
    private:
        int32_t m_windowId;
    };
    CORELIB_DECL_SHORTSPTR(EditorWindow);
}