#include "PropertyControls/PropertyControl.h"
#include <Pulsar/SceneObject.h>
#include <Pulsar/Assets/NodeCollection.h>
#include <PulsarEd/AssetDatabase.h>
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

    bool PropertyControlManager::ShowProperty(const string& name, Type* type, Object* obj, std::span<Attribute*> attrs)
    {
        auto ctrl = FindControl(type);
        bool changed = false;
        if (ctrl)
        {
            changed |= ctrl->OnDrawImGui(name, type, obj, attrs);
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
        Object* receiver, FieldInfo* receiverField = nullptr,
        bool ignore = false, bool showDebug = false,
        std::span<Attribute*> attrs = {})
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
        else
        {
            bool isInline = false;
            for (auto* attr : attrs)
            {
                if (attr && attr->GetType()->IsSubclassOf(cltypeof<InlineObjectAttribute>()))
                {
                    isInline = true;
                    break;
                }
            }

            if (!ignore && isInline && type->IsSubclassOf(cltypeof<PointerBoxingObject>()))
            {
                auto* boxing = dynamic_cast<PointerBoxingObject*>(obj);
                Object* innerObj = boxing ? boxing->GetPointer() : nullptr;
                if (innerObj)
                {
                    bool opened = ImGui::TreeNodeEx(StringUtil::FriendlyName(name).c_str());
                    ImGui::TableSetColumnIndex(1);
                    auto* innerBase = dynamic_cast<ObjectBase*>(innerObj);
                    ImGui::Text("%s", innerBase ? innerBase->GetName().c_str() : innerObj->GetType()->GetName().c_str());
                    if (opened)
                    {
                        isChanged |= _ObjectFieldPropertyLine(
                            name, innerObj->GetType(), innerObj->GetType(),
                            innerObj, receiver, receiverField, true, showDebug, attrs);
                    }
                    if (opened)
                    {
                        ImGui::TreePop();
                    }
                }
                else
                {
                    ImGui::TreeNodeEx(StringUtil::FriendlyName(name).c_str(), ImGuiTreeNodeFlags_Leaf);
                    ImGui::TableSetColumnIndex(1);
                    ImGui::Text("[Null]");
                    ImGui::TreePop();
                }
            }
            else if (PropertyControlManager::FindControl(innerType) && !ignore)
            {
                const bool opened = ImGui::TreeNodeEx(StringUtil::FriendlyName(name).c_str(), ImGuiTreeNodeFlags_Leaf);
                ImGui::TableSetColumnIndex(1);

                isChanged |= PropertyControlManager::ShowProperty(name, innerType, obj, attrs);

                if (opened)
                {
                    ImGui::TreePop();
                }
            }
            else
            {
                bool opened = ignore ? true : ImGui::TreeNodeEx(StringUtil::FriendlyName(name).c_str());

                if (opened)
                {
                    const auto fieldInfos = innerType->GetFieldInfos(TypeBinding::NonPublic);
                    for (int i = 0; i < fieldInfos.size(); ++i)
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

                        ImGui::PushID(i);

                        Object* parentObj = obj;
                        if (type->IsSubclassOf(cltypeof<PointerBoxingObject>()))
                        {
                            auto pointer = dynamic_cast<PointerBoxingObject*>(obj);
                            parentObj = pointer->GetPointer();
                        }
                        Object_sp fieldInstSptr = field->GetValue(parentObj);

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

                        auto fieldAttrsSptr = field->GetAllAttributes();
                        std::vector<Attribute*> fieldAttrs;
                        fieldAttrs.reserve(fieldAttrsSptr.size());
                        for (auto& a : fieldAttrsSptr)
                            fieldAttrs.push_back(a.get());

                        bool curFieldChanged = false;
                        if (curFieldChanged |= _ObjectFieldPropertyLine(
                                field->GetName(), field->GetFieldType(), fieldInnerType,
                                fieldInstSptr.get(), receiver, receiverField, false, showDebug, fieldAttrs))
                        {
                            if (fieldType->IsBoxingType())
                            {
                                field->SetValue(parentObj, fieldInstSptr);
                            }
                            if (auto* receiverBase = dynamic_cast<ObjectBase*>(receiver))
                            {
                                receiverBase->PostEditChange(receiverField);

                                // Mark owning scene dirty when editing scene objects
                                if (auto* sceneObj = dynamic_cast<SceneObject*>(receiverBase))
                                {
                                    if (auto* collection = sceneObj->GetOwnerNodeCollection())
                                    {
                                        if (auto* asset = ptr_cast<AssetObject>(collection))
                                        {
                                            auto rc = RCPtr<AssetObject>::UnsafeCreate(asset->GetObjectHandle());
                                            AssetDatabase::MarkDirty(rc);
                                        }
                                    }
                                }
                            }
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
    bool PImGui::PropertyLine(const string& name, Type* type, Object* obj, std::span<Attribute*> attrs)
    {
        bool changed = false;

        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();

        bool treeOpened = ImGui::TreeNodeEx(name.c_str(), ImGuiTreeNodeFlags_Leaf);

        ImGui::TableSetColumnIndex(1);
        ImGui::AlignTextToFramePadding();

        changed = PropertyControlManager::ShowProperty(name, type, obj, attrs);

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

    bool PImGui::ObjectFieldProperties(Type* type, Type* inner, Object* obj, Object* receiver, bool showDebug)
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