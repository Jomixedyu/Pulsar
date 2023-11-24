#pragma once
#include <pulsared/Assembly.h>
#include "EditorUI.h"

namespace pulsared
{
    class EditorWindow : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::EditorWindow, Object);

    public:
        virtual string_view GetWindowDisplayName() const { return "EditorWindow"; }
        virtual string GetWindowName() const { return GetType()->GetName(); }

        virtual void DrawImGui();
        virtual ImGuiWindowFlags GetGuiWindowFlags() const { return ImGuiWindowFlags_None; }
        EditorWindow();

    public:
        bool GetIsOpened() const { return m_isOpened; }
        int32_t GetWindowId() const { return this->m_windowId; }
    public:
        bool Open();
        void Close();
    protected:
        virtual void OnDrawImGui();
        virtual void OnOpen() {}
        virtual void OnClose() {}

    protected:
        bool m_isOpened = false;
        bool m_allowResize;
        Vector2f m_winSize;
    private:
        int32_t m_windowId;
    };
    CORELIB_DECL_SHORTSPTR(EditorWindow);
}