#include "MoveEdTool.h"

#include "EditorWorld.h"

#include <imgui/imgui.h>
#include "ImGuizmo.h"

#include "Pulsar/Components/CameraComponent.h"
#include "Pulsar/Node.h"


namespace pulsared
{
    void MoveEdTool::Begin()
    {
        base::Begin();
    }
    void MoveEdTool::Tick(float dt)
    {
        m_enableSelect = !ImGuizmo::IsOver();
        base::Tick(dt);

        if (GetSelection().IsEmpty())
        {
            return;
        }
        ImGuizmo::BeginFrame();
        ImGuizmo::SetOrthographic(false);
        ImGuizmo::SetDrawlist();

        Vector2f pos, size;
        GetViewportSize(pos, size);
        ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);

        auto viewMat = m_world->GetCurrentCamera()->GetViewMat();
        auto projMat = m_world->GetCurrentCamera()->GetProjectionMat();
        auto node = GetSelection().GetSelected();
        auto matrix = node->GetTransform()->GetLocalToWorldMatrix();

        static ImGuizmo::MODE CurrentGizmoMode(ImGuizmo::LOCAL);


        if (ImGuizmo::Manipulate(
            viewMat.get_value_ptr(),
            projMat.get_value_ptr(),
            ImGuizmo::TRANSLATE,
            CurrentGizmoMode,
            matrix.get_value_ptr(),
            nullptr, nullptr, nullptr, nullptr))
        {
            node->GetTransform()->SetWorldPosition(matrix[3].xyz());
        }

        // ImGuizmo::Manipulate(
        //     viewMat.get_value_ptr(),
        //     projMat.get_value_ptr(), mCurrentGizmoOperation, mCurrentGizmoMode, matrix.get_value_ptr(), NULL,
        // useSnap ? &snap[0] : NULL, boundSizing ? bounds : NULL, boundSizingSnap ? boundsSnap : NULL);
    }
} // namespace pulsared
