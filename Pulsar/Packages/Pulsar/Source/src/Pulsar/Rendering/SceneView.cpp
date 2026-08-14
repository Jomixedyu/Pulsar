#include "Rendering/SceneView.h"
#include "Rendering/RenderGraph/Pipelines/ViewPipeline.h"

#include <Pulsar/Application.h>

namespace pulsar
{
    SceneView::SceneView() = default;
    SceneView::~SceneView() = default;

    void SceneView::OnCreateResource()
    {
        Pipeline = std::make_unique<ViewPipeline>();
        Pipeline->OnCreateResource(Data.ViewPipeline);
    }

    void SceneView::OnDestroyResource()
    {
        Pipeline.reset();
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

    void SceneView::SetData(SceneViewData data)
    {
        Data = std::move(data);
        if (!Pipeline)
        {
            Pipeline = std::make_unique<ViewPipeline>();
            Pipeline->OnCreateResource(Data.ViewPipeline);
            return;
        }

        Pipeline->ApplyRenderData(Data.ViewPipeline);
    }
}
