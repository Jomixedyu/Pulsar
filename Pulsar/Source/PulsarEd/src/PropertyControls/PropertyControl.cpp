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
        const auto* mgr = manager();
        const auto it = mgr->find(type);
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
                if (list->GetCount() != 0)
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
                    ImGui::SameLine();
                    if (ImGui::Button(ICON_FK_ARROW_UP) && i != 0)
                    {
                        auto temp = list->At(i);
                        list->SetAt(i, list->At(i - 1));
                        list->SetAt(i - 1, temp);
                        isChanged |= true;
                    }
                    ImGui::SameLine();
                    if (ImGui::Button(ICON_FK_ARROW_DOWN) && i != list->GetCount() - 1)
                    {
                        auto temp = list->At(i);
                        list->SetAt(i, list->At(i + 1));
                        list->SetAt(i + 1, temp);
                        isChanged |= true;
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

            bool opened = ignore ? true : ImGui::TreeNodeEx(StringUtil::FriendlyName(name).c_str());

            if (opened)
            {
                const auto fieldInfos = innerType->GetFieldInfos(TypeBinding::NonPublic);
                for (size_t i = 0; i < fieldInfos.size(); ++i)
                {
                    const auto& field = fieldInfos[i];
                    Type* fieldType = field->GetFieldType();
                    if (field->IsDefinedAttribute(cltypeof<HidePropertyAttribute>()))
                    {
                        continue;
                    }
                    if (!showDebug && field->IsDefinedAttribute(cltypeof<DebugPropertyAttribute>()))
                    {
                        continue;
                    }

                    ImGui::PushID((int)i);
                    Object_sp fieldInstSptr;
                    Object* parentObj = obj;
                    if (type == BoxingObjectPtrBase::StaticType())
                    {
                        const auto objptr = dynamic_cast<BoxingObjectPtrBase*>(obj);
                        if (objptr->GetHandle())
                        {
                            parentObj = objptr->get_unboxing_value().GetObjectPointer();
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
                    else if (const auto* list = interface_cast<IList>(fieldInstSptr.get()))
                    {
                        fieldInnerType = list->GetIListElementType();
                    }

                    receiverField = ignore ? field : receiverField;

                    const bool isReadOnly = field->IsDefinedAttribute(cltypeof<ReadOnlyPropertyAttribute>());

                    if (isReadOnly)
                    {
                        ImGui::BeginDisabled();
                    }

                    bool curFieldChanged = false;
                    if (curFieldChanged |= _ObjectFieldPropertyLine(
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

                    if (isReadOnly)
                    {
                        ImGui::EndDisabled();
                    }

                    ImGui::PopID();
                }
            }
            if (opened && !ignore)
            {
                ImGui::TreePop();
            }
        }
        return isChanged;
    }
    constexpr auto _TableBorderFlags = ImGuiTableFlags_BordersV;
    bool PImGui::BeginPropertyLines()
    {
        bool b = ImGui::BeginTable("_lines", 2, _TableBorderFlags);
        if (b)
        {
            const float width = (float)ImGui::GetWindowWidth() * 0.38f;
            ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed, width);
            ImGui::TableSetupColumn("value");
        }
        return b;
    }
    bool PImGui::PropertyLine(const string& name, Type* type, Object* obj)
    {
        bool changed = false;

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();

        bool treeOpened = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Leaf);

        ImGui::TableSetColumnIndex(1);
        ImGui::AlignTextToFramePadding();

        changed = PropertyControlManager::ShowProperty(name, type, obj);

        if (treeOpened)
        {
            ImGui::TreePop();
        }

        return changed;
    }
    void PImGui::PropertyLineText(const string& name, string_view str)
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        bool treeOpened = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Leaf);

        ImGui::TableSetColumnIndex(1);
        ImGui::AlignTextToFramePadding();

        ImGui::Text(str.data());

        if (treeOpened)
        {
            ImGui::TreePop();
        }
    }
    void PImGui::EndPropertyLines()
    {
        ImGui::EndTable();
    }

    bool PImGui::ObjectFieldProperties(Type* type, Type* inner, Object* obj, ObjectBase* receiver, bool showDebug)
    {
        bool changed = false;
        if (ImGui::BeginTable("ss", 2, _TableBorderFlags))
        {
            const float width = (float)ImGui::GetWindowWidth() * 0.38f;
            ImGui::TableSetupColumn("name", ImGuiTableColumnFlags_WidthFixed, width);
            ImGui::TableSetupColumn("value");
            changed = _ObjectFieldPropertyLine("pp", type, inner, obj, receiver, nullptr, true, showDebug);
            ImGui::EndTable();
        }
        return changed;
    }
}