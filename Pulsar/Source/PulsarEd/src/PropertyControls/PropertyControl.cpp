#include "PropertyControls/PropertyControl.h"
#include <PulsarEd/EditorUI.h>
#include <imgui/imgui.h>

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
        const auto it = manager()->find(type);
        if (it != manager()->end())
        {
            return it->second;
        }
        if (it == manager()->end())
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

    bool PropertyControlManager::ShowProperty(const string& name, Type* type, Object* obj)
    {
        auto ctrl = FindControl(type);
        bool changed = false;
        if (ctrl)
        {
            changed |= ctrl->OnDrawImGui(name, type, obj);
        }
        else
        {
            ImGui::Text("not supported property.");
        }
        return changed;
    }

    bool PImGui::PropertyGroup(const char* label)
    {
        static bool t = true;
        return ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen);
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

    // type 禁止BoxingObjectPtrBase
    static bool _ObjectFieldPropertyLine(
        const string& name, Type* type, Type* innerType,
        Object* obj,
        ObjectBase* receiver, FieldInfo* receiverField = nullptr,
        bool ignore = false, bool showDebug = false)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();

        bool isChanged = false;
        if (type->IsImplementedInterface(cltypeof<IList>()))
        {
            const bool treeOpened = ImGui::TreeNodeEx(StringUtil::FriendlyName(name).c_str());
            ImGui::TableSetColumnIndex(1);

            auto* list = interface_cast<IList>(obj);
            Type* itemType = list->GetIListElementType();

            char text[32] = "List Count: ";
            StringUtil::strcat(text, std::to_string(list->GetCount()));

            ImGui::Text(text);
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_PLUS))
            {
                if (list->GetIListElementType() == BoxingObjectPtrBase::StaticType())
                {
                    ObjectPtrBase ptr{};
                    list->Add(mkbox(ptr));
                }
                else
                {
                    list->Add(list->GetIListElementType()->CreateSharedInstance({}));
                }

                isChanged = true;
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_MINUS))
            {
                if (list->GetCount() != 0)
                {
                    list->RemoveAt(list->GetCount() - 1);
                }
                isChanged = true;
            }
            ImGui::SameLine();
            if (ImGui::Button(ICON_FK_TRASH))
            {
                if(list->GetCount() != 0)
                {
                    list->Clear();
                    isChanged = true;
                }
            }

            if (treeOpened)
            {
                for (int i = 0; i < list->GetCount(); ++i)
                {
                    ImGui::PushID(i);
                    if (itemType->IsBoxingType())
                    {
                        auto boxing = list->At(i);
                        isChanged |= _ObjectFieldPropertyLine(std::to_string(i), itemType, innerType,
                            boxing.get(), receiver, receiverField, false, showDebug);
                        list->SetAt(i, boxing);
                    }
                    else
                    {
                        isChanged |= _ObjectFieldPropertyLine(std::to_string(i), itemType, innerType,
                            list->At(i).get(), receiver, receiverField, false, showDebug);
                    }
                    ImGui::PopID();
                }
                ImGui::TreePop();
            }
        }
        else if (PropertyControlManager::FindControl(innerType) && !ignore)
        {
            const bool opened = ImGui::TreeNodeEx(StringUtil::FriendlyName(name).c_str(), ImGuiTreeNodeFlags_Leaf);
            ImGui::TableSetColumnIndex(1);

            isChanged |= PropertyControlManager::ShowProperty(name, innerType, obj);

            if (opened)
            {
                ImGui::TreePop();
            }
        }
        else
        {
            // read fields
            const auto fieldInfos = innerType->GetFieldInfos(TypeBinding::NonPublic);
            for (size_t i = 0; i < fieldInfos.size(); ++i)
            {
                const auto& field = fieldInfos[i];
                Type* fieldType = field->GetFieldType();
                if (field->IsDefinedAttribute(cltypeof<HidePropertyAttribute>()))
                {
                    continue;
                }
                if(!showDebug && field->IsDefinedAttribute(cltypeof<DebugPropertyAttribute>()))
                {
                    continue;
                }

                ImGui::PushID((int)i);
                Object_sp fieldInstSptr;
                Object* parentObj = obj;
                if (type == BoxingObjectPtrBase::StaticType())
                {
                    const auto objptr = static_cast<BoxingObjectPtrBase*>(obj);
                    if (objptr->handle)
                    {
                        parentObj = objptr->get_unboxing_value().GetTPtr<ObjectBase>();
                        fieldInstSptr = field->GetValue(parentObj);
                    }
                    else
                    {
                        // fieldInstSptr = mkbox(ObjectPtrBase{});
                        fieldInstSptr = obj->shared_from_this();
                    }
                }
                else
                {
                    fieldInstSptr = field->GetValue(obj);
                }

                Type* fieldInnerType = field->GetWrapType() ? field->GetWrapType() : field->GetFieldType();
                if (const auto attr = field->GetAttribute<ListItemAttribute>())
                {
                    fieldInnerType = attr->GetItemType();
                }
                else if(const auto* list = interface_cast<IList>(fieldInstSptr.get()))
                {
                    fieldInnerType = list->GetIListElementType();
                }

                receiverField = ignore ? field : receiverField;

                const bool isReadOnly = field->IsDefinedAttribute(cltypeof<ReadOnlyPropertyAttribute>());

                if(isReadOnly)
                {
                    ImGui::BeginDisabled();
                }

                bool curFieldChanged = false;
                if(curFieldChanged |= _ObjectFieldPropertyLine(
                    field->GetName(), field->GetFieldType(), fieldInnerType,
                    fieldInstSptr.get(), receiver, receiverField, false, showDebug))
                {
                    if (fieldType->IsBoxingType())
                    {
                        field->SetValue(parentObj, fieldInstSptr);
                    }
                    receiver->PostEditChange(receiverField);
                }
                isChanged |= curFieldChanged;

                if(isReadOnly)
                {
                    ImGui::EndDisabled();
                }

                ImGui::PopID();
            }
        }
        return isChanged;
    }
    void PImGui::ObjectFieldProperties(Type* type, Type* inner, Object* obj, ObjectBase* receiver, bool showDebug)
    {
        if (ImGui::BeginTable("ss", 2, ImGuiTableFlags_BordersV))
        {
            const float width = (float)ImGui::GetWindowWidth() * 0.38f;
            ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed, width);
            ImGui::TableSetupColumn("value");
            _ObjectFieldPropertyLine("", type, inner, obj, receiver, nullptr, true, showDebug);
            ImGui::EndTable();
        }
    }
}