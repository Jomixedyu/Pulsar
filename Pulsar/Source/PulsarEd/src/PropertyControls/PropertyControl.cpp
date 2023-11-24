#include "PropertyControls/PropertyControl.h"
#include <imgui/imgui.h>
#include <PulsarEd/EditorUI.h>

namespace pulsared
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
        if (it != manager()->end())
        {
            for (auto& [k, v] : *manager())
            {
                if (type->IsSubclassOf(k))
                {
                    return v;
                }
            }

        }
        return nullptr;
    }

    static bool _DrawControl(const string& name, Type* type, Object* obj)
    {
        bool changed = false;
        auto prop_ctrl = PropertyControlManager::FindControl(type);
        if (prop_ctrl)
        {
            changed |= prop_ctrl->OnDrawImGui(name, obj);
        }
        else
        {
            ImGui::Text("not supported property.");
        }
        return changed;
    }

    bool PropertyControlManager::ShowProperty(const string& name, Type* type, Object* obj)
    {
        bool changed = false;
        if (type->IsImplementedInterface(cltypeof<IList>()))
        {
            IList* list = interface_cast<IList>(obj);

            ImGui::AlignTextToFramePadding();

            char text[64] = "List Count: ";
            itoa(list->GetCount(), text + 12, 10);

            ImGui::Text(text);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_PLUS))
            {
                list->Add(list->GetIListElementType()->CreateSharedInstance({}));
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_TRASH))
            {
                list->Clear();
            }


            for (int32_t i = 0; i < list->GetCount(); i++)
            {
                auto item = list->At(i);
                changed |= _DrawControl(name, list->GetIListElementType(), item.get());
            }

        }
        else
        {
            changed |= _DrawControl(name, type, obj);
        }

        return changed;
    }

    bool PImGui::PropertyGroup(const char* label)
    {
        return ImGui::CollapsingHeader(label);
    }

    bool PImGui::BeginPropertyItem(const char* name)
    {
        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, 120);
        ImGui::AlignTextToFramePadding();

        ImGui::Text(name);
        ImGui::NextColumn();
        return true;
    }

    void PImGui::EndPropertyItem()
    {
        ImGui::NextColumn();
        ImGui::Columns(1);
    }


    bool PImGui::PropertyLine(const string& name, Type* type, Object* obj)
    {
        BeginPropertyItem(StringUtil::FriendlyName(name).c_str());
        bool changed = PropertyControlManager::ShowProperty(name, type, obj);
        EndPropertyItem();

        return changed;
    }

    void PImGui::ObjectProperties(const string& name, Type* type, Object* obj)
    {
        IList* list = interface_cast<IList>(obj);
        
        if (auto ctrl = PropertyControlManager::FindControl(list ? list->GetIListElementType() : type))
        {
            BeginPropertyItem(StringUtil::FriendlyName(name).c_str());
            bool changed = PropertyControlManager::ShowProperty(name, type, obj);
            EndPropertyItem();
        }
        
        auto fieldinfos = type->GetFieldInfos(TypeBinding::NonPublic);
        for (size_t i = 0; i < fieldinfos.size(); i++)
        {
            auto& fieldinfo = fieldinfos[i];
            ImGui::PushID(i);
            ObjectProperties(fieldinfo->GetName(), fieldinfo->GetFieldType(), fieldinfo->GetValue(obj).get());
            ImGui::PopID();
        }

    }
}