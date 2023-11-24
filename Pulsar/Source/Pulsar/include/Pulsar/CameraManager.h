#pragma once
#include "Components/CameraComponent.h"

namespace pulsar
{
    class CameraManager
    {
    public:

        void AddCamera(CameraComponent_ref camera, bool isMainCamera = false);

        void RemoveCamera(CameraComponent_ref camera);
        void SetMainCamera(CameraComponent_ref camera)
        {
            m_mainCamera = camera;
        }
        CameraComponent_ref GetMainCamera() const
        {
            return m_mainCamera;
        }

        const array_list<CameraComponent_ref>& GetCameras() const { return m_cameras; }

    protected:
        array_list<CameraComponent_ref> m_cameras;
        CameraComponent_ref m_mainCamera;

    };
}