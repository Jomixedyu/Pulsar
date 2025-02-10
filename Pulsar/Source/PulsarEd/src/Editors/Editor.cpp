#include "Editors/Editor.h"

#include "Menus/Menu.h"
#include "Menus/MenuEntrySubMenu.h"

namespace pulsared
{

    void Editor::Initialize()
    {

    }

    void Editor::RegisterPanelType(Type* type)
    {
        if (type == nullptr)
        {
            return;
        }
        if (std::ranges::contains(m_registeredPanelTypes, type))
        {
            return;
        }
        if (!type->IsSubclassOf(cltypeof<PanelWindow>()))
        {
            return;
        }
        m_registeredPanelTypes.push_back(type);

        auto editorMenu = MenuManager::GetOrAddMenu(GetMenuName());
        auto menu = editorMenu->FindOrNewMenuEntry("Window");

        auto displayName = StringUtil::FriendlyName(type->GetShortName());

        auto checkAction = MenuCheckAction::FromLambda([](MenuContexts_sp ctxs, bool checked) {
            if (auto ctx = ctxs->FindContext<EditorWindowMenuContext>())
            {
                if (ctx->m_editorWindow)
                {
                    auto type = AssemblyManager::GlobalFindType(ctxs->EntryName);
                    if (checked)
                    {
                        ctx->m_editorWindow->OpenPanel(type);
                    }
                    else
                    {
                        ctx->m_editorWindow->ClosePanel(type);
                    }
                }
            }
        });
        auto getCheckAction = MenuGetCheckedAction::FromLambda([](MenuContexts_sp ctxs) {
            if (auto ctx = ctxs->FindContext<EditorWindowMenuContext>())
            {
                if (ctx->m_editorWindow)
                {
                    auto type = AssemblyManager::GlobalFindType(ctxs->EntryName);
                    return ctx->m_editorWindow->IsOpenedPanel(type);
                }
            }
            return false;
        });
        menu->AddEntry(mksptr(new MenuEntryCheck(type->GetName(), displayName, checkAction, getCheckAction)));
    }
    bool Editor::ContainsPanelType(Type* type) const
    {
        return std::ranges::contains(m_registeredPanelTypes, type);
    }
    SPtr<EditorWindow> Editor::CreateEditorWindow()
    {
        auto ptr = OnCreateEditorWindow();
        ptr->m_editor = this;
        return ptr;
    }
} // namespace pulsared
