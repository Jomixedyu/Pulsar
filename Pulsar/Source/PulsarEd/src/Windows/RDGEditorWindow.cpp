#include "ImGuiExt.h"

#include <PulsarEd/Windows/RDGEditorWindow.h>

#include <imgui/imgui.h>
#include <imgui_node_editor.h>

#define thisCtx reinterpret_cast<ax::NodeEditor::EditorContext*>(this->m_externalContext)

namespace pulsared
{

    void RDGEditorWindow::OnDrawImGui(float dt)
    {
        if (!m_graph)
            return;

        base::OnDrawImGui(dt);

        if (ImGui::IsKeyPressed(ImGuiKey_T, false))
        {
            m_graph->NewNode<RDGAttachmentNode>();
        }

        namespace ned = ax::NodeEditor;

        ned::SetCurrentEditor(thisCtx);
        ned::Begin("RDGGraphEditor");

        for (auto& node : m_graph->GetNodes())
        {
            ned::BeginNode((uintptr_t)node.get());

            ImGui::Text(node->GetTitle().data());

            auto pinRowCount = std::max(node->GetInPins().size(), node->GetOutPins().size());

            for (int i = 0; i < pinRowCount; ++i)
            {
                auto inPin = node->GetInPinAt(i);
                auto outPin = node->GetOutPinAt(i);

                if (inPin)
                {
                    ned::BeginPin((uintptr_t)inPin, (ax::NodeEditor::PinKind)inPin->m_type);
                    ImGui::Text(inPin->m_name.c_str());
                    ned::EndPin();
                }
                if (outPin)
                {
                    if (inPin)
                    {
                        ImGui::SameLine();
                        ImGui::Dummy(ImVec2(20, 0)); // Hacky magic number to space out the output pin.
                        ImGui::SameLine();
                    }
                    ned::BeginPin((uintptr_t)outPin, (ax::NodeEditor::PinKind)outPin->m_type);
                    ImGui::Text(outPin->m_name.c_str());
                    ned::EndPin();
                }


            }

            ned::EndNode();
        }

        for (auto& linked : m_graph->GetLinked())
        {
            ned::Link(linked.Id, (uintptr_t)linked.NodeA, (uintptr_t)linked.NodeB);
        }

        // ==================================================================================================
        // Interaction Handling Section
        // This was coppied from BasicInteration.cpp. See that file for commented code.

        // Handle creation action ---------------------------------------------------------------------------
        if (ned::BeginCreate())
        {
            ned::PinId inputPinId, outputPinId;
            if (ned::QueryNewLink(&inputPinId, &outputPinId))
            {
                auto input = (GraphPin*)inputPinId.Get();
                auto output = (GraphPin*)outputPinId.Get();

                if (inputPinId && outputPinId)
                {
                    if (ned::AcceptNewItem())
                    {
                        m_graph->MakeLink(input, output);

                        // m_Links.push_back({ ned::LinkId(m_NextLinkId++), inputPinId, outputPinId });
                        // ned::Link(m_Links.back().Id, m_Links.back().InputId, m_Links.back().OutputId);
                    }
                }
            }
            ned::EndCreate();
        }

        // Handle deletion action ---------------------------------------------------------------------------
        if (ned::BeginDelete())
        {
            ned::LinkId deletedLinkId;
            while (ned::QueryDeletedLink(&deletedLinkId))
            {
                if (ned::AcceptDeletedItem())
                {
                    m_graph->BreakLinkById(deletedLinkId.Get());
                }
            }
            ned::EndDelete();
        }

        ned::End();
        ned::SetCurrentEditor(nullptr);
    }
    void RDGEditorWindow::OnOpen()
    {
        base::OnOpen();
        m_externalContext = ax::NodeEditor::CreateEditor();
        m_graph = cref_cast<RDGGraph>(m_assetObject);
    }
    void RDGEditorWindow::OnClose()
    {
        base::OnClose();
        DestroyEditor(thisCtx);
    }

    void RDGEditorWindow::OnRefreshMenuContexts()
    {
        base::OnRefreshMenuContexts();
    }
} // namespace pulsared