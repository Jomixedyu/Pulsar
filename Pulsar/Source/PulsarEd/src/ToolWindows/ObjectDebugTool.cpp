#include "AssetDatabase.h"

#include <PulsarEd/Menus/Menu.h>
#include <PulsarEd/ToolWindows/ObjectDebugTool.h>

namespace pulsared
{

    ObjectDebugTool::ObjectDebugTool()
    {
        m_winSize = {800,500};
    }
    void ObjectDebugTool::OnDrawImGui(float dt)
    {
        ImGui::InputText("Search", m_search, sizeof(m_search));

        ImGui::BeginTable("tab", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable);
        ImGui::TableSetupColumn("Object Name");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Object Handle");
        ImGui::TableSetupColumn("Persistent Path");
        ImGui::TableHeadersRow();

        RuntimeObjectWrapper::ForEachObject([](ObjectHandle handle, ObjectBase* obj) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text(obj->GetName().c_str());

            ImGui::TableSetColumnIndex(1);
            ImGui::Text(obj->GetType()->GetName().c_str());

            ImGui::TableSetColumnIndex(2);
            ImGui::Text(handle.to_string().c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text(AssetDatabase::GetPathById(handle).c_str());
        });
        ImGui::EndTable();
    }
} // namespace pulsared