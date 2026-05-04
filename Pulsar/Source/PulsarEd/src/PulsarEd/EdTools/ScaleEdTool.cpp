#include "ScaleEdTool.h"

#include "EditorWorld.h"

#include <imgui/imgui.h>
#include "ImGuizmo.h"

#include "Pulsar/Components/CameraComponent.h"
#include "Pulsar/Node.h"
#include <PulsarEd/AssetDatabase.h>


namespace pulsared
{
    void ScaleEdTool::Begin()
    {
        base::Begin();
    }

    void ScaleEdTool::Tick(float dt)
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

        auto T_i = jmath::Inverse(jmath::Translate(node->GetTransform()->GetWorldPosition()));
        auto R_i = jmath::Inverse(jmath::Rotate(node->GetTransform()->GetRotation()));

        auto mode = static_cast<ImGuizmo::MODE>(m_world->GetGizmoSpace());

        if (ImGuizmo::Manipulate(
            viewMat.get_value_ptr(),
            projMat.get_value_ptr(),
            ImGuizmo::SCALE,
            mode,
            matrix.get_value_ptr(),
            nullptr, nullptr, nullptr, nullptr))
        {
            auto S = R_i * T_i * matrix;
            Vector3f scale {S[0][0], S[1][1], S[2][2]};
            node->GetTransform()->SetScale(scale);
            if (auto focusScene = m_world->GetFocusScene())
            {
                if (auto asset = cast<AssetObject>(focusScene))
                {
                    AssetDatabase::MarkDirty(asset);
                }
            }
        }


    }
} // namespace pulsared