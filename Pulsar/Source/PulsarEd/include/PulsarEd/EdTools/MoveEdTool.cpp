#include "MoveEdTool.h"

#include "EditorSelection.h"
#include <imgui/imgui.h>
#include "ImGuizmo.h"
#include "Pulsar/Components/CameraComponent.h"
#include "Pulsar/Node.h"

namespace pulsared
{
    static ImGuizmo::OPERATION mCurrentGizmoOperation(ImGuizmo::TRANSLATE);
    static ImGuizmo::MODE mCurrentGizmoMode(ImGuizmo::LOCAL);

    void MoveEdTool::Begin()
    {
        base::Begin();
        Logger::Log("MoveTool");
    }
    void MoveEdTool::Tick(float dt)
    {
        base::Tick(dt);
        if (EditorSelection::Selection.IsEmpty())
        {
            return;
        }
        ImGuizmo::BeginFrame();
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        static bool useSnap = false;
        static float snap[3] = { 1.f, 1.f, 1.f };
        static float bounds[] = { -0.5f, -0.5f, -0.5f, 0.5f, 0.5f, 0.5f };
        static float boundsSnap[] = { 0.1f, 0.1f, 0.1f };
        static bool boundSizing = false;
        static bool boundSizingSnap = false;

        float windowWidth = (float)ImGui::GetWindowWidth();
        float windowHeight = (float)ImGui::GetWindowHeight();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

        auto viewMat = m_world->GetPreviewCamera()->GetViewMat();
        auto projMat = m_world->GetPreviewCamera()->GetProjectionMat();
        auto node = EditorSelection::Selection.GetSelected();
        auto matrix = node->GetTransform()->GetLocalToWorldMatrix();

        ImGuizmo::Manipulate(
            viewMat.get_value_ptr(),
            projMat.get_value_ptr(), mCurrentGizmoOperation, mCurrentGizmoMode, matrix.get_value_ptr(), NULL,
        useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);

        //ImGuizmo::ViewManipulate(viewMat.get_value_ptr(), 1, ImVec2(2, 0), ImVec2(128, 128), 0x10101010);
    }
} // namespace pulsared
