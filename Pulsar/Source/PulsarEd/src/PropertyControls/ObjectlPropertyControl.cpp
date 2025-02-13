#include "AssetDatabase.h"
#include "DragInfo.h"
#include "PropertyControls/ObjectPropertyControl.h"
#include <Pulsar/AssetManager.h>
#include <Pulsar/IconsForkAwesome.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>

namespace PImGui
{

}

namespace pulsared
{


    bool ObjectPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop)
    {
        bool isChanged = false;
        auto objectPtr = dynamic_cast<BoxingObjectPtrBase*>(prop);
        auto rcobjPtr = dynamic_cast<BoxingRCPtrBase*>(prop);

        if (!objectPtr && !rcobjPtr)
        {
            throw EngineException();
        }
        ObjectHandle handle;
        if (objectPtr)
        {
            handle = objectPtr->GetHandle();
        }
        else
        {
            handle = rcobjPtr->GetHandle();
        }
        auto SetHandle = [=](ObjectHandle handle)
        {
            if (objectPtr)
            {
                objectPtr->SetHandle(handle);
            }
            else
            {
                rcobjPtr->SetHandle(handle);
            }
        };

        char buf[128] = "[Null Object Reference]";
        enum { eNullRef = 0x01, eObject = 0x02, eMissing = 0x04 } mode = eNullRef;

        string objectName;
        if (!handle.is_empty())
        {
            auto obj = RuntimeObjectManager::GetObject(handle);
            if (!obj)
            {
                obj = AssetDatabase::LoadAssetById(handle).GetPtr();
            }
            auto objectShortHandle = handle.to_string().substr(24, 8);
            if (obj)
            {
                objectName = StringUtil::Concat(obj->GetName(), " (", objectShortHandle, ")");
                StringUtil::strcpy(buf, objectName);
                mode = eObject;
            }
            else
            {
                StringUtil::strcpy(buf, StringUtil::Concat("[Missing Object (", objectShortHandle, ")]"));
                mode = eMissing;
            }
        }

        ImGui::InputTextEx("##i", nullptr, buf, sizeof(buf), ImVec2(-50, 0), 0);

        if (ImGui::BeginDragDropTarget())
        {
            const ImGuiPayload* payload = ImGui::GetDragDropPayload();
            string dragType;

            if (payload->DataType == ObjectPtrDragInfo::Name)
            {
                const auto dragData = static_cast<ObjectPtrDragInfo*>(payload->Data);

                if (dragData->Type->IsSubclassOf(type))
                {
                    if ((payload = ImGui::AcceptDragDropPayload(ObjectPtrDragInfo::Name.data())))
                    {
                        (void)payload;
                        SetHandle(dragData->ObjectHandle);
                        isChanged = true;
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }

        return isChanged;
    }
} // namespace pulsared