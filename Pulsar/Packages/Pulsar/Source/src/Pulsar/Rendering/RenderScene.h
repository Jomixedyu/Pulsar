#pragma once
#include "PerRenderObjectDataManager.h"
#include "RenderObject.h"
#include "RenderProxy.h"
#include "SceneView.h"

namespace pulsar
{
    // Render-thread-exclusive container of the render-side scene state.
    // Owns every RenderProxy (primitive proxies + view proxies) and the per-object
    // data manager. All mutation must happen on the render thread (driven by enqueued
    // commands); the *_RenderThread methods are the only sanctioned mutators.
    class RenderScene
    {
    public:
        RenderScene() = default;
        ~RenderScene() = default;

        RenderScene(const RenderScene&) = delete;
        RenderScene& operator=(const RenderScene&) = delete;

        // Unified proxy registry. Dispatches by proxy type into the matching container.
        void AddProxy_RenderThread(SPtr<rendering::RenderProxy> proxy);
        void RemoveProxy_RenderThread(const SPtr<rendering::RenderProxy>& proxy);

        // Destroy the whole scene on the render thread. Driven by the dedicated
        // destroy channel (RenderThread::EnqueueDestroy_AnyThread), not the update queue.
        void Destroy_RenderThread();

        const hash_set<rendering::RenderObject_sp>& GetRenderObjects() const { return m_renderObjects; }
        const array_list<SPtr<SceneView>>& GetViews() const { return m_views; }
        PerRenderObjectDataManager& GetPerRenderObjectData() { return m_perObjectData; }

    private:
        void AddRenderObject(const rendering::RenderObject_sp& ro);
        void RemoveRenderObject(const rendering::RenderObject_sp& ro);
        void AddView(const SPtr<SceneView>& view);
        void RemoveView(const SPtr<SceneView>& view);

        hash_set<rendering::RenderObject_sp> m_renderObjects;
        array_list<SPtr<SceneView>>          m_views;
        PerRenderObjectDataManager           m_perObjectData;
    };
}
