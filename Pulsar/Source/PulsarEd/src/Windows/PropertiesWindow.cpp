#include "EditorWorld.h"

#include <PulsarEd/Windows/PropertiesWindow.h>

namespace pulsared
{
    
    static auto properties_panel()
    {
        static auto m = new map<Type*, Type*>;
        return m;
    }

    static Type* _GetSelectionType()
    {
        auto world = dynamic_cast<EditorWorld*>(EditorWorld::GetPreviewWorld());
        for (auto& item : *properties_panel())
        {
            if (world->GetSelection().IsType(item.first))
            {
                return item.second;
            }
        }
        return nullptr;
    }

    void PropertiesWindow::OnDrawImGui(float dt)
    {
        auto select_type = _GetSelectionType();

        if (this->current_panel == nullptr && select_type)
        {
            this->current_panel = sptr_cast<PropertiesPanel>(select_type->CreateSharedInstance({}));
        }
        else if (select_type == nullptr)
        {
            this->current_panel = nullptr;
        }


        if (this->current_panel)
        {
            this->current_panel->OnDrawImGui();
        }
    }
    void PropertiesWindow::OnOpen()
    {
        base::OnOpen();
    }
    void PropertiesWindow::OnClose()
    {
        base::OnClose();
    }

    void PropertiesWindow::StaticRegisterPropertiesPanel(Type* target_type, Type* panel_type)
    {
        properties_panel()->emplace(target_type, panel_type);
    }


}