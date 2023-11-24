#include <Pulsar/CameraManager.h>

namespace pulsar
{
    void CameraManager::AddCamera(CameraComponent_ref camera, bool isMainCamera)
    {
        m_cameras.push_back(camera);
        if (isMainCamera || m_cameras.size() == 1)
        {
            SetMainCamera(camera);
        }
    }

    void CameraManager::RemoveCamera(CameraComponent_ref camera)
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