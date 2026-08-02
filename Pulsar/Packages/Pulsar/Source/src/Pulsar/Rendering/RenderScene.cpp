#include "Rendering/RenderScene.h"

#include <Pulsar/Application.h>
#include <Pulsar/Rendering/RenderThread.h>
#include <gfx/GFXResourceManager.h>

namespace pulsar
{
    // Asserts the caller is on the render thread (or that no render thread exists yet,
    // e.g. during shutdown after the thread was torn down -> synchronous fallback).
    static void AssertRenderThread()
    {
        auto* rt = Application::GetRenderThread();
        assert((!rt || rt->IsRenderThread()) && "RenderScene must be mutated on the render thread");
    }

    void RenderScene::AddProxy_RenderThread(SPtr<rendering::RenderProxy> proxy)
    {
        AssertRenderThread();
        if (!proxy)
            return;

        if (auto ro = std::dynamic_pointer_cast<rendering::RenderObject>(proxy))
        {
            AddRenderObject(ro);
        }
        else if (auto view = std::dynamic_pointer_cast<SceneView>(proxy))
        {
            AddView(view);
        }
        else if (auto point = std::dynamic_pointer_cast<PointLightProxy>(proxy))
        {
            point->OnCreateResource();
            m_pointLights.push_back(point);
        }
        else if (auto dir = std::dynamic_pointer_cast<DirectionalLightProxy>(proxy))
        {
            dir->OnCreateResource();
            m_directionalLights.push_back(dir);
        }
    }

    void RenderScene::RemoveProxy_RenderThread(const SPtr<rendering::RenderProxy>& proxy)
    {
        AssertRenderThread();
        if (!proxy)
            return;

        if (auto ro = std::dynamic_pointer_cast<rendering::RenderObject>(proxy))
        {
            RemoveRenderObject(ro);
        }
        else if (auto view = std::dynamic_pointer_cast<SceneView>(proxy))
        {
            RemoveView(view);
        }
        else if (auto point = std::dynamic_pointer_cast<PointLightProxy>(proxy))
        {
            const auto it = std::ranges::find(m_pointLights, point);
            if (it != m_pointLights.end())
            {
                (*it)->OnDestroyResource();
                m_pointLights.erase(it);
            }
        }
        else if (auto dir = std::dynamic_pointer_cast<DirectionalLightProxy>(proxy))
        {
            const auto it = std::ranges::find(m_directionalLights, dir);
            if (it != m_directionalLights.end())
            {
                (*it)->OnDestroyResource();
                m_directionalLights.erase(it);
            }
        }
    }

    void RenderScene::AddRenderObject(const rendering::RenderObject_sp& ro)
    {
        auto slot = m_perObjectData.AllocSlot();
        ro->SetRenderObjectIndex(slot);
        ro->SetPerRenderObjectDataManager(&m_perObjectData);
        m_perObjectData.SetData(slot, ro->GetPerRenderObjectData());
        ro->OnCreateResource();
        m_renderObjects.insert(ro);
    }

    void RenderScene::RemoveRenderObject(const rendering::RenderObject_sp& ro)
    {
        const auto it = m_renderObjects.find(ro);
        if (it == m_renderObjects.end())
            return;

        (*it)->OnDestroyResource();
        const auto slot = (*it)->GetRenderObjectIndex();
        if (slot != rendering::RenderObject::kInvalidSlot)
        {
            m_perObjectData.FreeSlot(slot);
        }
        m_renderObjects.erase(it);
    }

    void RenderScene::AddView(const SPtr<SceneView>& view)
    {
        view->OnCreateResource();
        m_views.push_back(view);
    }

    void RenderScene::RemoveView(const SPtr<SceneView>& view)
    {
        const auto it = std::ranges::find(m_views, view);
        if (it == m_views.end())
            return;
        (*it)->OnDestroyResource();
        m_views.erase(it);
    }

    void RenderScene::Destroy_RenderThread()
    {
        AssertRenderThread();
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
        m_worldBuffer = gfxApp->CreateBuffer(desc);

        desc.BufferSize = sizeof(PerPassLightsBufferData);
        m_lightsBuffer = gfxApp->CreateBuffer(desc);
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
