#include "Rendering/SceneView.h"
#include "Rendering/RenderGraph/ScriptableCaptureRenderer.h"

#include <Pulsar/Application.h>

namespace pulsar
{
    SceneView::~SceneView() = default;

    void SceneView::OnDestroyResource()
    {
        m_cameraBuffer.reset();
    }

    void SceneView::UploadCamera(const PerPassCameraData& data)
    {
        if (!m_cameraBuffer)
        {
            gfx::GFXBufferDesc desc{};
            desc.Usage = gfx::GFXBufferUsage::ConstantBuffer;
            desc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnDevice;
            desc.BufferSize = sizeof(PerPassCameraData);
            m_cameraBuffer = Application::GetGfxApp()->CreateBuffer(desc);
        }
        if (m_cameraBuffer)
            m_cameraBuffer->Update(&data);
    }

    gfx::GFXBuffer* SceneView::GetCameraBuffer() const
    {
        return m_cameraBuffer.get();
    }
}
