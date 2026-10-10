#include "Rendering/RenderScene.h"
#include "Rendering/RenderGraph/Pipelines/SceneRenderPipeline.h"

#include <Pulsar/Application.h>
#include <Pulsar/Rendering/RenderThread.h>
#include <gfx/GFXResourceRegistry.h>

namespace pulsar
{
    RenderScene::RenderScene() = default;
    RenderScene::~RenderScene() = default;

    // Asserts the caller is on the render thread (or that no render thread exists yet,
    // e.g. during shutdown after the thread was torn down -> synchronous fallback).
    static void AssertRenderThread()
    {
        auto* rt = Application::GetRenderThread();
        assert((!rt || rt->IsRenderThread()) && "RenderScene must be mutated on the render thread");
    }

    SceneRenderPipeline& RenderScene::GetRenderPipeline()
    {
        AssertRenderThread();
        if (!m_renderPipeline)
            m_renderPipeline = std::make_unique<SceneRenderPipeline>();
        return *m_renderPipeline;
    }
    void RenderScene::AddProxy(SPtr<rendering::SceneProxy> proxy)
    {
        AssertRenderThread();
        if (!proxy)
            return;

        if (auto ro = std::dynamic_pointer_cast<rendering::RenderObjectProxy>(proxy))
        {
            AddRenderObject(ro);
        }
        else if (auto view = std::dynamic_pointer_cast<SceneViewProxy>(proxy))
        {
            AddView(view);
        }
        else if (auto point = std::dynamic_pointer_cast<PointLightProxy>(proxy))
        {
            point->OnEnterScene();
            m_pointLights.push_back(point);
        }
        else if (auto dir = std::dynamic_pointer_cast<DirectionalLightProxy>(proxy))
        {
            dir->OnEnterScene();
            m_directionalLights.push_back(dir);
        }
    }

    void RenderScene::RemoveProxy(const SPtr<rendering::SceneProxy>& proxy)
    {
        AssertRenderThread();
        if (!proxy)
            return;

        if (auto ro = std::dynamic_pointer_cast<rendering::RenderObjectProxy>(proxy))
        {
            RemoveRenderObject(ro);
        }
        else if (auto view = std::dynamic_pointer_cast<SceneViewProxy>(proxy))
        {
            RemoveView(view);
        }
        else if (auto point = std::dynamic_pointer_cast<PointLightProxy>(proxy))
        {
            const auto it = std::ranges::find(m_pointLights, point);
            if (it != m_pointLights.end())
            {
                (*it)->OnLeaveScene();
                m_pointLights.erase(it);
            }
        }
        else if (auto dir = std::dynamic_pointer_cast<DirectionalLightProxy>(proxy))
        {
            const auto it = std::ranges::find(m_directionalLights, dir);
            if (it != m_directionalLights.end())
            {
                (*it)->OnLeaveScene();
                m_directionalLights.erase(it);
            }
        }
    }

    void RenderScene::AddRenderObject(const rendering::RenderObjectProxy_sp& ro)
    {
        auto slot = m_perObjectData.AllocSlot();
        ro->SetRenderObjectIndex(slot);
        ro->SetPerRenderObjectDataManager(&m_perObjectData);
        m_perObjectData.SetData(slot, ro->GetPerRenderObjectData());
        ro->OnEnterScene();
        m_renderObjects.insert(ro);
    }

    void RenderScene::RemoveRenderObject(const rendering::RenderObjectProxy_sp& ro)
    {
        const auto it = m_renderObjects.find(ro);
        if (it == m_renderObjects.end())
            return;

        (*it)->OnLeaveScene();
        const auto slot = (*it)->GetRenderObjectIndex();
        if (slot != rendering::RenderObjectProxy::kInvalidSlot)
        {
            m_perObjectData.FreeSlot(slot);
        }
        m_renderObjects.erase(it);
    }

    void RenderScene::AddView(const SPtr<SceneViewProxy>& view)
    {
        view->OnEnterScene();
        m_views.push_back(view);
    }

    void RenderScene::RemoveView(const SPtr<SceneViewProxy>& view)
    {
        const auto it = std::ranges::find(m_views, view);
        if (it == m_views.end())
            return;
        (*it)->OnLeaveScene();
        m_views.erase(it);
    }

    void RenderScene::Destroy()
    {
        AssertRenderThread();
        m_renderPipeline.reset();
        m_views.clear();
        m_pointLights.clear();
        m_directionalLights.clear();
        m_renderObjects.clear();
        m_perObjectData.Destroy();

        m_worldBuffer.reset();
        m_lightsBuffer.reset();
    }

    void RenderScene::EnsureBuffers()
    {
        if (m_worldBuffer)
            return;

        auto* gfxApp = Application::GetGfxApp();
        gfx::GFXBufferDesc desc{};
        desc.Usage = gfx::GFXBufferUsage::ConstantBuffer;
        desc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnDevice;

        desc.BufferSize = sizeof(PerPassWorldData);
        m_worldBuffer = gfxApp->GetResourceRegistry()->CreateBuffer(desc);

        desc.BufferSize = sizeof(PerPassLightsBufferData);
        m_lightsBuffer = gfxApp->GetResourceRegistry()->CreateBuffer(desc);
    }

    void RenderScene::UploadWorld(const PerPassWorldData& data)
    {
        EnsureBuffers();
        if (m_worldBuffer)
            m_worldBuffer->Update(&data);
    }

    void RenderScene::UploadLights(const PerPassLightsBufferData& data)
    {
        EnsureBuffers();
        if (m_lightsBuffer)
            m_lightsBuffer->Update(&data);
    }

    gfx::GFXBuffer* RenderScene::GetWorldBuffer() const
    {
        return m_worldBuffer.get();
    }

    gfx::GFXBuffer* RenderScene::GetLightsBuffer() const
    {
        return m_lightsBuffer.get();
    }
}
