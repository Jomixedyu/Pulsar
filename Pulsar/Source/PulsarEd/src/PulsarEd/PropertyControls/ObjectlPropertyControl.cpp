#include "AssetDatabase.h"
#include "DragInfo.h"
#include "PropertyControls/ObjectPropertyControl.h"
#include <Pulsar/AssetManager.h>
#include <Pulsar/Node.h>
#include <Pulsar/IconsForkAwesome.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace PImGui
{

}

namespace pulsared
{

    static bool AssetObjectControl(BoxingRCPtrBase* obj, Type* type)
    {
        auto ptr = obj->ptr;

        char buf[128] = "[Null AssetObject Reference]";

        string objectName;
        if (ptr.IsValid())
        {
            auto guid = ptr.GetGuid();
            auto shortGuid = guid.to_string().substr(24, 8);
            objectName = StringUtil::Concat(ptr->GetName(), " (", shortGuid, ")");

            StringUtil::strcpy(buf, objectName);
        }
        else if (!ptr.IsEmpty())
        {
            StringUtil::strcpy(buf, StringUtil::Concat("[Missing Object)]"));
        }

        ImGui::InputTextEx("##i", nullptr, buf, sizeof(buf), ImVec2(-50, 0), 0);
        bool isChanged = false;
        if (ImGui::BeginDragDropTarget())
        {
            const ImGuiPayload* payload = ImGui::GetDragDropPayload();
            string dragType;

            if (payload->DataType == AssetObjectDragInfo::Name)
            {
                const auto dragData = static_cast<AssetObjectDragInfo*>(payload->Data);

                if (dragData->Type->IsSubclassOf(type))
                {
                    if ((payload = ImGui::AcceptDragDropPayload(AssetObjectDragInfo::Name.data())))
                    {
                        (void)payload;
                        //obj->ptr = RuntimeAssetManager::GetLoadedAssetByGuid(dragData->AssetGuid);
                        obj->ptr = AssetDatabase::LoadAssetById(dragData->AssetGuid);
                        isChanged = true;
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }
        return isChanged;
    }
    static bool SceneObjectControl(BoxingSceneObjectPtrBase* obj, Type* type)
    {
        auto ptr = cast<SceneObject>(obj->ptr);

        char buf[128] = "[Null Object Reference]";

        string objectName;
        if (ptr.IsValid())
        {
            auto guid = ptr->GetSceneObjectGuid();
            auto shortGuid = guid.to_string().substr(24, 8);

            if (auto component = dynamic_cast<Component*>(ptr.GetPtr()))
            {
                auto node = component->GetNode();
                string nodeName = node ? node->GetName() : string("<No Node>");
                string nodeShortGuid = node ? node->GetSceneObjectGuid().to_string().substr(24, 8) : string("--------");
                string componentShortGuid = component->GetSceneObjectGuid().to_string().substr(24, 8);
                string friendlyComponentName = ComponentInfoManager::GetFriendlyComponentName(component->GetType());
                objectName = StringUtil::Concat(nodeName, " (", nodeShortGuid, ") | ", friendlyComponentName, " (", componentShortGuid, ")");
            }
            else
            {
                objectName = StringUtil::Concat(ptr->GetName(), " (", shortGuid, ")");
            }

            StringUtil::strcpy(buf, objectName);
        }
        else if (!ptr.IsEmpty())
        {
            StringUtil::strcpy(buf, StringUtil::Concat("[Missing Object]"));
        }

        ImGui::InputTextEx("##i", nullptr, buf, sizeof(buf), ImVec2(-50, 0), 0);
        bool isChanged = false;
        if (ImGui::BeginDragDropTarget())
        {
            const ImGuiPayload* payload = ImGui::GetDragDropPayload();
            string dragType;

            if (payload->DataType == SceneObjectDragInfo::Name)
            {
                const auto dragData = static_cast<SceneObjectDragInfo*>(payload->Data);

                if (dragData->Type->IsSubclassOf(type))
                {
                    if ((payload = ImGui::AcceptDragDropPayload(SceneObjectDragInfo::Name.data())))
                    {
                        (void)payload;

                        obj->ptr = dragData->Handle;
                        isChanged = true;
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }
        return isChanged;
    }

    bool ObjectPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop, std::span<Attribute*> attrs)
    {
        if (auto sceneObjectPtr = dynamic_cast<BoxingSceneObjectPtrBase*>(prop))
        {
            return SceneObjectControl(sceneObjectPtr, type);
        }

        if (dynamic_cast<BoxingObjectPtrBase*>(prop))
        {
            return false;
        }

        if (auto rcobjPtr = dynamic_cast<BoxingRCPtrBase*>(prop))
        {
            return AssetObjectControl(rcobjPtr, type);
        }

        return false;
    }
} // namespace pulsared