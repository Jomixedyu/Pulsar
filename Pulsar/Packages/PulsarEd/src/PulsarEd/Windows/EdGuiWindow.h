#pragma once
#include <pulsared/Assembly.h>
#include "EditorUI.h"

namespace pulsared
{
    class EdGuiWindow : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::EdGuiWindow, Object);

    public:
        virtual string_view GetWindowDisplayName() const { return m_windowDisplayName; }
        virtual string GetWindowName() const { return GetType()->GetName(); }

        virtual void DrawImGui(float dt);
        virtual ImGuiWindowFlags GetGuiWindowFlags() const { return ImGuiWindowFlags_None; }
        virtual ImGuiWindowClass GetGuiWindowClass() const { return ImGuiWindowClass{}; }
        EdGuiWindow();

    public:
        bool GetIsOpened() const { return m_isOpened; }
        int32_t GetWindowId() const { return this->m_windowId; }
    public:
        bool Open();
        void Close();
    protected:
        virtual void OnDrawImGui(float dt);
        virtual void OnOpen() {}
        virtual void OnClose() {}

    protected:
        bool m_isOpened = false;
        bool m_allowResize;
        Vector2f m_winSize;
        string m_windowDisplayName = "EditorWindow";
    private:
        int32_t m_windowId;
    };
    CORELIB_DECL_SHORTSPTR(EdGuiWindow);
}