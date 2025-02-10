#include "ImGuiExt.h"


#include <PulsarEd/Editors/RDGEditor//RDGEditorWindow.h>

#include <imgui/imgui.h>
#include <imgui_node_editor.h>
#include <utilities/builders.h>
#include "utilities/drawing.h"
#include "utilities/widgets.h"

#define thisCtx reinterpret_cast<ax::NodeEditor::EditorContext*>(this->m_externalContext)

namespace pulsared
{
    enum class NodeType
    {
        Blueprint,
        Simple,
        Tree,
        Comment,
        Houdini
    };
    // ImColor GetIconColor(PinType type)
    // {
    //     switch (type)
    //     {
    //     default:
    //     case PinType::Flow:     return ImColor(255, 255, 255);
    //     case PinType::Bool:     return ImColor(220,  48,  48);
    //     case PinType::Int:      return ImColor( 68, 201, 156);
    //     case PinType::Float:    return ImColor(147, 226,  74);
    //     case PinType::String:   return ImColor(124,  21, 153);
    //     case PinType::Object:   return ImColor( 51, 150, 215);
    //     case PinType::Function: return ImColor(218,   0, 183);
    //     case PinType::Delegate: return ImColor(255,  48,  48);
    //     }
    // };
    void DrawPinIcon(GraphPin* pin, bool connected, int alpha)
    {
        ax::Drawing::IconType iconType;
        ImColor  color = ImColor(255, 255, 255, alpha);
        // ImColor  color = GetIconColor(pin.Type);
        color.Value.w = alpha / 255.0f;
        // switch (pin.Type)
        // {
        // case PinType::Flow:     iconType = IconType::Flow;   break;
        // case PinType::Bool:     iconType = IconType::Circle; break;
        // case PinType::Int:      iconType = IconType::Circle; break;
        // case PinType::Float:    iconType = IconType::Circle; break;
        // case PinType::String:   iconType = IconType::Circle; break;
        // case PinType::Object:   iconType = IconType::Circle; break;
        // case PinType::Function: iconType = IconType::Circle; break;
        // case PinType::Delegate: iconType = IconType::Square; break;
        // default:
        //     return;
        // }
        iconType = ax::Drawing::IconType::Circle;
        ax::Widgets::Icon(ImVec2(static_cast<float>(24), static_cast<float>(24)), iconType, connected, color, ImColor(32, 32, 32, alpha));
    };
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

        // for (auto& node : m_graph->GetNodes())
        // {
        //     ned::BeginNode((uintptr_t)node.get());
        //
        //     ImGui::Text(node->GetTitle().data());
        //
        //     auto pinRowCount = std::max(node->GetInPins().size(), node->GetOutPins().size());
        //
        //     for (int i = 0; i < pinRowCount; ++i)
        //     {
        //         auto inPin = node->GetInPinAt(i);
        //         auto outPin = node->GetOutPinAt(i);
        //
        //         if (inPin)
        //         {
        //             ned::BeginPin((uintptr_t)inPin, (ax::NodeEditor::PinKind)inPin->m_type);
        //             ImGui::Text(inPin->m_name.c_str());
        //             ned::EndPin();
        //         }
        //         if (outPin)
        //         {
        //             if (inPin)
        //             {
        //                 ImGui::SameLine();
        //                 ImGui::Dummy(ImVec2(20, 0)); // Hacky magic number to space out the output pin.
        //                 ImGui::SameLine();
        //             }
        //             ned::BeginPin((uintptr_t)outPin, (ax::NodeEditor::PinKind)outPin->m_type);
        //             ImGui::Text(outPin->m_name.c_str());
        //             ned::EndPin();
        //         }
        //
        //
        //     }
        //
        //     ned::EndNode();
        // }

        namespace util = ax::NodeEditor::Utilities;

        util::BlueprintNodeBuilder builder(0, 0, 0);

           for (auto& node : m_graph->GetNodes())
            {

                const auto isSimple = false;

                builder.Begin((ned::NodeId)node.get());
               // title
                if (!isSimple)
                {
                    builder.Header(ImColor(0.8f, 0.2f, 0.2f));
                        ImGui::Spring(0);
                        ImGui::TextUnformatted(node->GetTitle().data());
                        ImGui::Spring(1);
                        ImGui::Dummy(ImVec2(0, 28));

                            ImGui::Spring(0);
                    builder.EndHeader();
                }

                for (auto& input : node->GetInPins())
                {
                    auto alpha = ImGui::GetStyle().Alpha;
                    // if (newLinkPin && !CanCreateLink(newLinkPin, &input) && &input != newLinkPin)
                    //     alpha = alpha * (48.0f / 255.0f);

                    builder.Input((ned::PinId)input.get());
                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                    DrawPinIcon(input.get(), !input->m_linked.empty(), (int)(alpha * 255));
                    ImGui::Spring(0);
                    if (!input->m_name.empty())
                    {
                        ImGui::TextUnformatted(input->m_name.c_str());
                        ImGui::Spring(0);
                    }

                    ImGui::PopStyleVar();
                    builder.EndInput();
                }

                for (auto& output : node->GetOutPins())
                {
                    auto alpha = ImGui::GetStyle().Alpha;
                    // if (newLinkPin && !CanCreateLink(newLinkPin, &output) && &output != newLinkPin)
                    //     alpha = alpha * (48.0f / 255.0f);

                    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
                    builder.Output((ned::PinId)output.get());

                    if (!output->m_name.empty())
                    {
                        ImGui::Spring(0);
                        ImGui::TextUnformatted(output->m_name.c_str());
                    }
                    ImGui::Spring(0);
                    DrawPinIcon(output.get(), !output->m_linked.empty(), (int)(alpha * 255));
                    ImGui::PopStyleVar();
                    builder.EndOutput();
                }

                builder.End();
            }



        for (auto& linked : m_graph->GetLinked())
        {
            ned::Link(linked.Id, (uintptr_t)linked.NodeA, (uintptr_t)linked.NodeB);
        }

        // Handle creation action ---------------------------------------------------------------------------
        if (ned::BeginCreate())
        {
            ned::PinId aPinId, bPinId;
            if (ned::QueryNewLink(&aPinId, &bPinId))
            {
                auto aInput = (GraphPin*)aPinId.Get();
                auto bInput = (GraphPin*)bPinId.Get();

                if (aPinId && bPinId)
                {
                    if ((aInput->m_type == GraphPinType::Input && bInput->m_type == GraphPinType::Output) ||
                        (aInput->m_type == GraphPinType::Output && bInput->m_type == GraphPinType::Input))
                    {
                        if (ned::AcceptNewItem())
                        {
                            m_graph->MakeLink(aInput, bInput);
                        }
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