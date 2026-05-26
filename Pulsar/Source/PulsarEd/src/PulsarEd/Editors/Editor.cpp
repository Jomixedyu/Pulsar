#include "Editors/Editor.h"

#include "Editors/EditorWindow.h"
#include "Menus/Menu.h"
#include "Menus/MenuEntrySubMenu.h"
#include "Windows/PanelWindow.h"

namespace pulsared
{

    void Editor::Initialize()
    {

    }

    SPtr<EditorWindow> Editor::CreateEditorWindow()
    {
        auto ptr = OnCreateEditorWindow();
        ptr->m_editor = this;
        return ptr;
    }
} // namespace pulsared
