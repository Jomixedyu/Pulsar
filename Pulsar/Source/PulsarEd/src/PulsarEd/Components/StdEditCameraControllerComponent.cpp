#include "Components/StdEditCameraControllerComponent.h"

#include "Subsystems/EditorGridSubsystem.h"

namespace pulsared
{

    StdEditCameraControllerComponent::StdEditCameraControllerComponent()
    {
        m_flags |= OF_NoPack;
        m_saved3d.ProjectionMode = CaptureProjectionMode::Perspective;
        m_saved2d.ProjectionMode = CaptureProjectionMode::Orthographic;
        m_saved2d.CamPos.z = -100;
    }

    bool StdEditCameraControllerComponent::CanRotate() const
    {
        return !m_enable2DMode;
    }

    void StdEditCameraControllerComponent::SetEnable2DMode(bool enable2DMode)
    {
        m_enable2DMode = enable2DMode;
        if (auto grid = GetWorld()->GetSubsystem<EditorGridSubsystem>())
        {
            if (enable2DMode)
            {
                grid->SetGridMode(EditorGridSubsystem::e2D);
            }
            else
            {
                grid->SetGridMode(EditorGridSubsystem::e3D);
            }
        }
    }

    void StdEditCameraControllerComponent::BeginComponent()
    {
        base::BeginComponent();
        if (auto grid = GetWorld()->GetSubsystem<EditorGridSubsystem>())
        {
            grid->SetGridMode(m_enable2DMode ? EditorGridSubsystem::e2D : EditorGridSubsystem::e3D);
        }
    }

    void StdEditCameraControllerComponent::EndComponent()
    {
        base::EndComponent();
    }
} // namespace pulsared