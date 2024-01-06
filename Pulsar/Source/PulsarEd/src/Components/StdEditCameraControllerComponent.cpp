#include "Components/StdEditCameraControllerComponent.h"

namespace pulsared
{

    StdEditCameraControllerComponent::StdEditCameraControllerComponent()
    {
        m_saved3d.ProjectionMode = CameraProjectionMode::Perspective;
        m_saved2d.ProjectionMode = CameraProjectionMode::Orthographic;
        m_saved2d.CamPos.z = -100;
    }
} // namespace pulsared