#include "Rendering/RenderScene.h"

#include <Pulsar/Application.h>
#include <Pulsar/Rendering/RenderThread.h>

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
        m_renderObjects.clear();
        m_perObjectData.Destroy();
    }
}
