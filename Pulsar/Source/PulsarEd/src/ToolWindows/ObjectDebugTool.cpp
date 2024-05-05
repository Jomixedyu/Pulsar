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

        size_t total, pending, alive;
        RuntimeObjectManager::GetData(&total, &pending, &alive);
        ImGui::Text("Total Obejct: %d, Pending Kill: %d, Alive: %d", (int)total, (int)pending, (int)alive);

        ImGui::BeginTable("tab", 6, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable);
        ImGui::TableSetupColumn("Object Name");
        ImGui::TableSetupColumn("Type");
        ImGui::TableSetupColumn("Object Handle");
        ImGui::TableSetupColumn("Persistent Path");
        ImGui::TableSetupColumn("RCCounter");
        ImGui::TableSetupColumn("PtrUseCounter");
        ImGui::TableHeadersRow();

        RuntimeObjectManager::ForEachObject([](auto& info) {
            ImGui::TableNextRow();

            ImGui::TableSetColumnIndex(0);
            ImGui::Text(info.Pointer ? info.Pointer->GetName().c_str() : "/* PENDING KILL */");

            ImGui::TableSetColumnIndex(1);
            ImGui::Text(info.Pointer ? info.Pointer->GetType()->GetName().c_str() : "");

            ImGui::TableSetColumnIndex(2);
            ImGui::Text(info.Handle.to_string().c_str());

            ImGui::TableSetColumnIndex(3);
            ImGui::Text(AssetDatabase::GetPathById(info.Handle).c_str());

            ImGui::TableSetColumnIndex(4);
            ImGui::Text(std::to_string(RuntimeObjectManager::GetPointer(info.Handle)->RefCount()).c_str());

            ImGui::TableSetColumnIndex(5);
            ImGui::Text(std::to_string(info.ManagedCounter).c_str());
        });
        ImGui::EndTable();
    }
} // namespace pulsared