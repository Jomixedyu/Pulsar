#pragma once
#include "Windows/EdGuiWindow.h"
#include "Windows/PanelWindow.h"

namespace pulsared
{
    class ISubMenu;
}
namespace pulsared
{
    class Editor;
    class EditorWindow;

    class EditorWindowMenuContext : public MenuContextBase
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::EditorWindowMenuContext, MenuContextBase);
    public:

        EditorWindow* m_editorWindow = nullptr;
    };

    class EditorWindow : public EdGuiWindow
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::EditorWindow, EdGuiWindow);
        friend class Editor;
    public:

        ImGuiWindowFlags GetGuiWindowFlags() const override;
        ImGuiWindowClass GetGuiWindowClass() const override;

        void OnOpen() override;
        void OnClose() override;

        void OnDrawImGui(float dt) override;
        void DrawImGui(float dt) override;

        void OpenPanel(Type* type);
        void ClosePanel(Type* type);
        bool IsOpenedPanel(Type* type) const;

        Editor* GetEditor() const { return m_editor; }
    protected:
        virtual void OnPanelStateChanged(PanelWindow* win, bool open);
    private:
        void OnGlobalPanelStateChanged(EdGuiWindow* win, bool opened);
    protected:
        bool m_useDockspace = true;
    private:
        Editor* m_editor = nullptr;
        array_list<SPtr<PanelWindow>> m_openedPanels;
        ImGuiID m_dockspaceClassId{};
    };
}