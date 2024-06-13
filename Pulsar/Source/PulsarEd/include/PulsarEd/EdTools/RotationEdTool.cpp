#include "RotationEdTool.h"

#include "EditorWorld.h"

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

    template<typename T>
    jmath::Quaternion<T> MatToQuat(const jmath::Matrix4<T>& m)
    {
        T fourXSquaredMinus1 = m[0][0] - m[1][1] - m[2][2];
        T fourYSquaredMinus1 = m[1][1] - m[0][0] - m[2][2];
        T fourZSquaredMinus1 = m[2][2] - m[0][0] - m[1][1];
        T fourWSquaredMinus1 = m[0][0] + m[1][1] + m[2][2];

        int biggestIndex = 0;
        T fourBiggestSquaredMinus1 = fourWSquaredMinus1;
        if(fourXSquaredMinus1 > fourBiggestSquaredMinus1)
        {
            fourBiggestSquaredMinus1 = fourXSquaredMinus1;
            biggestIndex = 1;
        }
        if(fourYSquaredMinus1 > fourBiggestSquaredMinus1)
        {
            fourBiggestSquaredMinus1 = fourYSquaredMinus1;
            biggestIndex = 2;
        }
        if(fourZSquaredMinus1 > fourBiggestSquaredMinus1)
        {
            fourBiggestSquaredMinus1 = fourZSquaredMinus1;
            biggestIndex = 3;
        }

        T biggestVal = sqrt(fourBiggestSquaredMinus1 + static_cast<T>(1)) * static_cast<T>(0.5);
        T mult = static_cast<T>(0.25) / biggestVal;

        switch(biggestIndex)
        {
        case 0:
            return jmath::Quaternion<T>((m[1][2] - m[2][1]) * mult, (m[2][0] - m[0][2]) * mult, (m[0][1] - m[1][0]) * mult, biggestVal);
        case 1:
            return jmath::Quaternion<T>(biggestVal, (m[0][1] + m[1][0]) * mult, (m[2][0] + m[0][2]) * mult, (m[1][2] - m[2][1]) * mult);
        case 2:
            return jmath::Quaternion<T>((m[0][1] + m[1][0]) * mult, biggestVal, (m[1][2] + m[2][1]) * mult, (m[2][0] - m[0][2]) * mult);
        case 3:
            return jmath::Quaternion<T>((m[2][0] + m[0][2]) * mult, (m[1][2] + m[2][1]) * mult, biggestVal, (m[0][1] - m[1][0]) * mult);
        default: // Silence a -Wswitch-default warning in GCC. Should never actually get here. Assert is just for sanity.
            assert(false);
            return jmath::Quaternion<T>(0, 0, 0, 1);
        }
    }

    void RotationEdTool::Tick(float dt)
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

        auto windowWidth = (float)ImGui::GetWindowWidth();
        auto windowHeight = (float)ImGui::GetWindowHeight();
        ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

        auto viewMat = m_world->GetCurrentCamera()->GetViewMat();
        auto projMat = m_world->GetCurrentCamera()->GetProjectionMat();
        auto node = GetSelection().GetSelected();
        auto matrix = node->GetTransform()->GetLocalToWorldMatrix();

        static ImGuizmo::MODE CurrentGizmoMode(ImGuizmo::LOCAL);
        auto T_i = jmath::Inverse(jmath::Translate(node->GetTransform()->GetWorldPosition()));
        auto S_i = jmath::Inverse(jmath::Scale(node->GetTransform()->GetWorldScale()));

        if (ImGuizmo::Manipulate(
            viewMat.get_value_ptr(),
            projMat.get_value_ptr(),
            ImGuizmo::ROTATE,
            CurrentGizmoMode,
            matrix.get_value_ptr(),
            nullptr, nullptr, nullptr, nullptr))
        {
            float translate[3], rotation[3], scale[3];
            ImGuizmo::DecomposeMatrixToComponents(matrix.get_value_ptr(), translate, rotation, scale);
            auto R = T_i * matrix * S_i;
            Logger::Log(to_string(T_i));
            Logger::Log(to_string(matrix));
            Logger::Log(to_string(S_i));
            node->GetTransform()->SetEuler(Vector3f{rotation[0], rotation[1], rotation[2]});
            // auto q = MatToQuat(R);
            // node->GetTransform()->SetEuler();
        }


    }
} // namespace pulsared