#include "RotationEdTool.h"

#include "EditorWorld.h"

#include <imgui/imgui.h>
#include "ImGuizmo.h"
#include "Pulsar/Components/CameraComponent.h"
#include "Pulsar/Node.h"
#include <PulsarEd/AssetDatabase.h>

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

        Vector2f pos, size;
        GetViewportSize(pos, size);
        ImGuizmo::SetRect(pos.x, pos.y, size.x, size.y);

        auto viewMat = m_world->GetCurrentCamera()->GetViewMat();
        auto projMat = m_world->GetCurrentCamera()->GetProjectionMat();
        auto node = GetSelection().GetSelected();
        auto matrix = node->GetTransform()->GetLocalToWorldMatrix();

        auto mode = static_cast<ImGuizmo::MODE>(m_world->GetGizmoSpace());

        if (ImGuizmo::Manipulate(
            viewMat.get_value_ptr(),
            projMat.get_value_ptr(),
            ImGuizmo::ROTATE,
            mode,
            matrix.get_value_ptr(),
            nullptr, nullptr, nullptr, nullptr))
        {
            auto transform = node->GetTransform();

            // Normalize each column of the world matrix to strip scale, leaving pure rotation
            Matrix4f rotMat = matrix;
            for (int col = 0; col < 3; ++col)
            {
                float len = jmath::Magnitude(Vector3f{rotMat[col][0], rotMat[col][1], rotMat[col][2]});
                if (len > 1e-6f)
                {
                    rotMat[col][0] /= len;
                    rotMat[col][1] /= len;
                    rotMat[col][2] /= len;
                }
            }
            auto worldQuat = MatToQuat(rotMat);

            // Remove parent world rotation to get local rotation:
            // local = Inverse(parentWorldRot) * worldRot
            auto parentMat = transform->GetParentLocalToWorldMatrix();
            Matrix4f parentRotMat = parentMat;
            for (int col = 0; col < 3; ++col)
            {
                float len = jmath::Magnitude(Vector3f{parentRotMat[col][0], parentRotMat[col][1], parentRotMat[col][2]});
                if (len > 1e-6f)
                {
                    parentRotMat[col][0] /= len;
                    parentRotMat[col][1] /= len;
                    parentRotMat[col][2] /= len;
                }
            }
            auto parentWorldQuat = MatToQuat(parentRotMat);

            transform->SetRotation(jmath::Inverse(parentWorldQuat) * worldQuat);
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