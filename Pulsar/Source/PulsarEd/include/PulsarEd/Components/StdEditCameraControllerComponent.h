#pragma once
#include "EditorComponent.h"
#include "Pulsar/Components/CameraComponent.h"

namespace pulsared
{
	class StdEditCameraControllerComponent : public EditorComponent
	{
		CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::StdEditCameraControllerComponent, EditorComponent);
	public:
        StdEditCameraControllerComponent();

	    struct CameraState
	    {
	        Vector3f ControllerPos;
	        Vector3f ControllerEuler;
	        Vector3f CamPos;
	        Vector3f CamEuler;
	        CameraProjectionMode ProjectionMode;
	    };

	    CameraState m_saved3d{};
	    CameraState m_saved2d{};

	    bool m_enable2DMode = false;
	};
}