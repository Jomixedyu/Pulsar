#include "AssetDatabase.h"
#include "PropertyControls/ObjectPropertyControl.h"
#include <Pulsar/IconsForkAwesome.h>
#include <imgui/imgui.h>
#include <imgui/imgui_internal.h>
#include <Pulsar/AssetManager.h>

namespace PImGui
{

}

namespace pulsared
{


    bool ObjectPropertyControl::OnDrawImGui(const string& name, Type* type, Object* prop)
    {
        bool isChanged = false;
        auto objectPtr = dynamic_cast<BoxingObjectPtrBase*>(prop);

        if(!objectPtr)
        {
            throw EngineException();
        }

        char buf[128] = "Null";

        string objectName;
        if(!objectPtr->handle.is_empty())
        {
            auto obj = RuntimeObjectWrapper::GetObject(objectPtr->handle);
            if(!obj)
            {
                obj = AssetDatabase::LoadAssetById(objectPtr->handle).GetPtr();
            }
            auto objectShortHandle = objectPtr->handle.to_string().substr(24, 8);
            if(obj)
            {
                objectName = StringUtil::Concat(obj->GetName(), " (", objectShortHandle , ")");
                StringUtil::strcpy(buf, objectName);
            }
            else
            {
                StringUtil::strcpy(buf, StringUtil::Concat("missing object (", objectShortHandle, ")"));
            }
        }

        ImGui::InputTextEx("##i", nullptr, buf, sizeof(buf), ImVec2(-50, 0), 0);

        if(ImGui::BeginDragDropTarget())
        {
            const ImGuiPayload* payload = ImGui::GetDragDropPayload();
            string dragType;

            if(!std::strcmp(payload->DataType, "PULSARED_DRAG"))
            {
                const auto str = string_view( static_cast<char*>(payload->Data), payload->DataSize );

                auto strs = StringUtil::Split(str, ";");
                auto& intype = strs[0];
                auto& id = strs[1];

                if(intype == type->GetName())
                {
                    if(payload = ImGui::AcceptDragDropPayload("PULSARED_DRAG"))
                    {
                        objectPtr->handle = ObjectHandle::parse(id);
                        TryFindOrLoadObject(objectPtr->handle);
                        isChanged = true;
                    }
                }
            }

            ImGui::EndDragDropTarget();
        }

        return isChanged;
    }
} // namespace pulsared