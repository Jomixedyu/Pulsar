#include "PropertyControl.h"
#include <ThirdParty/imgui/imgui.h>

namespace apatiteed
{


    static inline auto manager()
    {
        static auto inst = new map<Type*, PropertyControl*>;
        return inst;
    }
    void PropertyControlManager::RegisterControl(Type* type, PropertyControl* control)
    {
        manager()->emplace(type, control);
    }
    PropertyControl* PropertyControlManager::FindControl(Type* type)
    {
        auto it = manager()->find(type);
        if (it == manager()->end())
        {
            return nullptr;
        }
        return it->second;
    }
    void PropertyControlManager::ShowProperty(const string& name, const sptr<Object>& obj)
    {
        if (obj->GetType()->IsImplementedInterface(cltypeof<IList>()))
        {
            IList* list = interface_cast<IList>(obj.get());
            for (int32_t i = 0; i < list->GetCount(); i++)
            {
                auto item = list->At(i);
                auto prop_ctrl = PropertyControlManager::FindControl(item->GetType());
                if (prop_ctrl)
                {
                    prop_ctrl->OnDrawImGui(name, item);
                }
                else
                {
                    ImGui::Text("not supported property.");
                }
            }
        }
        else
        {
            auto prop_ctrl = PropertyControlManager::FindControl(obj->GetType());
            if (prop_ctrl)
            {
                prop_ctrl->OnDrawImGui(name, obj);
            }
            else
            {
                ImGui::Text("not supported property.");
            }
        }
    }
}