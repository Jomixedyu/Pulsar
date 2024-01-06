#include <Pulsar/CameraManager.h>

namespace pulsar
{
    void CameraManager::AddCamera(ObjectPtr<CameraComponent> camera, bool isMainCamera)
    {
        m_cameras.push_back(camera);
        if (isMainCamera || m_cameras.size() == 1)
        {
            SetMainCamera(camera);
        }
    }

    void CameraManager::RemoveCamera(ObjectPtr<CameraComponent> camera)
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
}