#pragma once
#include <pulsared/Assembly.h>
#include "EditorUI.h"
#include <PulsarEd/Menus/MenuEntry.h>

namespace pulsared
{
    class EditorWindow;

    class PanelWindow : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::PanelWindow, Object);
    public:
        virtual string_view GetWindowDisplayName() const { return m_windowDisplayName; }
        virtual string GetWindowName() const { return GetType()->GetName(); }
        virtual ImGuiWindowFlags GetGuiWindowFlags() const { return ImGuiWindowFlags_NoCollapse; }
        virtual ImGuiWindowClass GetGuiWindowClass() const;

        virtual void OnDrawImGui(float dt) {}
        virtual void OnOpen() {}
        virtual void OnClose() {}

        bool GetIsOpened() const { return m_isOpened; }
        void SetOpened(bool opened) { m_isOpened = opened; }

        Vector2f GetWinSize() const { return m_winSize; }
        void SetWinSize(const Vector2f& size) { m_winSize = size; }

        bool GetAllowResize() const { return m_allowResize; }
        void SetAllowResize(bool allow) { m_allowResize = allow; }

        int GetParentWindowId() const { return m_parentWindowId; }
        EditorWindow* GetParentEditorWindow() const { return m_parentEditorWindow; }

        int m_parentWindowId = 0;
        EditorWindow* m_parentEditorWindow = nullptr;
    protected:
        MenuEntryCheck_sp GetCheckedEntry() const;
        string m_windowDisplayName = "PanelWindow";
        bool m_isOpened = false;
        bool m_allowResize = true;
        Vector2f m_winSize = Vector2f{50, 50};
    };
    CORELIB_DECL_SHORTSPTR(PanelWindow);
}
