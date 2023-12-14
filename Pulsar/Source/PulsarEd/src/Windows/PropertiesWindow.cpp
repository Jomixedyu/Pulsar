#include <PulsarEd/Windows/PropertiesWindow.h>
#include <PulsarEd/EditorSelection.h>

namespace pulsared
{
    
    static auto properties_panel()
    {
        static auto m = new map<Type*, Type*>;
        return m;
    }

    static Type* _GetSelectionType()
    {
        for (auto& item : *properties_panel())
        {
            if (EditorSelection::Selection.IsType(item.first))
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