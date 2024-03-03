#include "RotationEdTool.h"
#include "EditorSelection.h"
#include <imgui/imgui.h>
#include "ImGuizmo.h"
#include "Pulsar/Components/CameraComponent.h"
#include "Pulsar/Node.h"

namespace pulsared
{
    void RotationEdTool::Begin()
    {
        base::Begin();
    }
    void RotationEdTool::Tick(float dt)
    {
        m_enableSelect = !ImGuizmo::IsOver();
        base::Tick(dt);

        if (EditorSelection::Selection.IsEmpty())
        {
            return;
        }
        ImGuizmo::BeginFrame();
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        auto windowWidth = (float)ImGui::GetWindowWidth();
        auto windowHeight = (float)ImGui::GetWindowHeight();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

        auto viewMat = m_world->GetPreviewCamera()->GetViewMat();
        auto projMat = m_world->GetPreviewCamera()->GetProjectionMat();
        auto node = EditorSelection::Selection.GetSelected();
        auto matrix = node->GetTransform()->GetLocalToWorldMatrix();

        static ImGuizmo::MODE CurrentGizmoMode(ImGuizmo::WORLD);
        auto T_i = jmath::Inverse(jmath::Translate(node->GetTransform()->GetWorldPosition()));
        auto S_i = jmath::Inverse(jmath::Scale(node->GetTransform()->GetScale()));

        if (ImGuizmo::Manipulate(
            viewMat.get_value_ptr(),
            projMat.get_value_ptr(),
            ImGuizmo::ROTATE,
            CurrentGizmoMode,
            matrix.get_value_ptr(),
            nullptr, nullptr, nullptr, nullptr))
        {
            auto R = T_i * matrix * S_i;

            // node->GetTransform()->SetRotation()
        }


    }
} // namespace pulsared