#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/Menus/MenuEntry.h>
#include <PulsarEd/Menus/MenuEntrySubMenu.h>
#include <PulsarEd/Menus/MenuRenderer.h>
#include <PulsarEd/PropertyControls/PropertyControl.h>
#include <PulsarEd/Shaders/EditorShader.h>
#include <PulsarEd/Windows/MaterialEditorWindow.h>

namespace pulsared
{

    void MaterialEditorWindow::OnOpen()
    {
        base::OnOpen();
    }
    void MaterialEditorWindow::OnClose()
    {
        base::OnClose();
    }



    void MaterialEditorWindow::OnRefreshMenuContexts()
    {
        base::OnRefreshMenuContexts();
        // m_menuBarCtxs->Contexts.push_back();
    }
} // namespace pulsared