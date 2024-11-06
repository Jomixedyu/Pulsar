#include <Pulsar/CameraManager.h>
#include "Components/CameraComponent.h"

namespace pulsar
{
    void CameraManager::AddCamera(const ObjectPtr<CameraComponent>& camera, bool isMainCamera)
    {
        m_cameras.push_back(camera);
        if (isMainCamera || m_cameras.size() == 1)
        {
            SetMainCamera(camera);
        }
    }

    void CameraManager::RemoveCamera(const ObjectPtr<CameraComponent>& camera)
    {
        auto it = std::find(m_cameras.begin(), m_cameras.end(), camera);
        if (it != m_cameras.end())
        {
            m_cameras.erase(it);
        }
        if (camera == m_mainCamera)
        {
            SetMainCamera(nullptr);
        }
    }
    void CameraManager::SetMainCamera(const ObjectPtr<CameraComponent>& camera)
    {
        m_mainCamera = camera;
    }


} // namespace pulsar