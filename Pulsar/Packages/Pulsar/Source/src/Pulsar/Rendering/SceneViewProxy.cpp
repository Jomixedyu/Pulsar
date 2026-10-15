#include "Rendering/SceneViewProxy.h"
#include "Rendering/RenderGraph/Pipelines/ViewPipeline.h"

#include <Pulsar/Application.h>
#include <gfx/GFXResourceRegistry.h>

namespace pulsar
{
    SceneViewProxy::SceneViewProxy() = default;
    SceneViewProxy::~SceneViewProxy() = default;

    void SceneViewProxy::OnEnterScene()
    {
        Pipeline = std::make_unique<ViewPipeline>();
        Pipeline->OnCreateResource(Data.ViewPipeline);
    }

    void SceneViewProxy::OnLeaveScene()
    {
        Pipeline.reset();
        m_cameraBuffer.reset();
    }

    void SceneViewProxy::UploadCamera(const PerPassCameraData& data)
    {
        if (!m_cameraBuffer)
        {
            gfx::GFXBufferDesc desc{};
            desc.Usage = gfx::GFXBufferUsage::ConstantBuffer;
            desc.StorageType = gfx::GFXBufferMemoryPosition::DeviceLocal;
            desc.BufferSize = sizeof(PerPassCameraData);
            m_cameraBuffer = Application::GetGfxApp()->GetResourceRegistry()->CreateBuffer(desc);
        }
        if (m_cameraBuffer)
            m_cameraBuffer->Update(&data);
    }

    gfx::GFXBuffer* SceneViewProxy::GetCameraBuffer() const
    {
        return m_cameraBuffer.get();
    }

    void SceneViewProxy::SetData(SceneViewData data)
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
