#pragma once
#include "Assembly.h"
#include "Pulsar/Components/CameraComponent.h"

namespace pulsared
{
	class StdEditCameraControllerComponent : public Component
	{
		CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::StdEditCameraControllerComponent, Component);
	public:
        StdEditCameraControllerComponent();

	    struct CameraState
	    {
	        Vector3f ControllerPos;
	        Vector3f ControllerEuler;
	        Vector3f CamPos;
	        Vector3f CamEuler;
	        CaptureProjectionMode ProjectionMode;
	    };

	    bool CanRotate() const;

	    CameraState m_saved3d{};
	    CameraState m_saved2d{};

	    void SetEnable2DMode(bool enable2DMode);
	    bool GetEnable2DMode() const { return m_enable2DMode; }

	    void BeginComponent() override;
	    void EndComponent() override;

	private:

	    bool m_enable2DMode = false;
	};
}