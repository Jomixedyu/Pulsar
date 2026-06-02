#include "EditorRegistry.h"

namespace pulsared
{
    array_list<SPtr<Editor>> EditorRegistry::s_editors;

    void EditorRegistry::Register(Type* editorType)
    {
        GetRegistrations().push_back(editorType);
    }

    void EditorRegistry::Initialize()
    {
        for (auto* type : GetRegistrations())
        {
            auto editor = sptr_cast<Editor>(type->CreateSharedInstance({}));
            if (editor)
            {
                s_editors.push_back(std::move(editor));
            }
        }
        for (auto& editor : s_editors)
        {
            editor->Initialize();
        }
    }

    void EditorRegistry::Terminate()
    {
        for (auto& editor : s_editors)
        {
            editor->Terminate();
        }
        s_editors.clear();
    }

    array_list<SPtr<Editor>>& EditorRegistry::GetEditors()
    {
        return s_editors;
    }

    Editor* EditorRegistry::GetEditor(Type* editorType)
    {
        for (auto& editor : s_editors)
        {
            if (editor->GetType() == editorType)
                return editor.get();
        }
        return nullptr;
    }

    void EditorRegistry::AddEditor(SPtr<Editor> editor)
    {
        editor->Initialize();
        s_editors.push_back(std::move(editor));
    }

    void RegisterEditorType(Type* editorType)
    {
        EditorRegistry::Register(editorType);
    }
}
