#pragma once
#include "PerRenderObjectDataManager.h"
#include "PerPassData.h"
#include "RenderObject.h"
#include "RenderProxy.h"
#include "LightProxy.h"
#include "SceneView.h"
#include <gfx/GFXBuffer.h>

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
        const array_list<SPtr<PointLightProxy>>& GetPointLights() const { return m_pointLights; }
        const array_list<SPtr<DirectionalLightProxy>>& GetDirectionalLights() const { return m_directionalLights; }
        PerRenderObjectDataManager& GetPerRenderObjectData() { return m_perObjectData; }

        // World-level time snapshot. Written on the render thread via the update queue
        // (World pushes it at end of Tick), read by renderers. Avoids touching live World.
        void SetTime_RenderThread(float totalTime, float deltaTime) { m_totalTime = totalTime; m_deltaTime = deltaTime; }
        float GetTotalTime() const { return m_totalTime; }
        float GetDeltaTime() const { return m_deltaTime; }

        // Scene-wide per-pass cbuffers (owned here; camera lives on SceneView).
        // Buffers are lazily created on first upload (render thread, GFX alive).
        void UploadWorld(const PerPassWorldData& data);
        void UploadLights(const PerPassLightsBufferData& data);
        gfx::GFXBuffer* GetWorldBuffer() const;
        gfx::GFXBuffer* GetLightsBuffer() const;

    private:
        void AddRenderObject(const rendering::RenderObject_sp& ro);
        void RemoveRenderObject(const rendering::RenderObject_sp& ro);
        void AddView(const SPtr<SceneView>& view);
        void RemoveView(const SPtr<SceneView>& view);

        void EnsureBuffers();

        hash_set<rendering::RenderObject_sp> m_renderObjects;
        array_list<SPtr<SceneView>>          m_views;
        array_list<SPtr<PointLightProxy>>       m_pointLights;
        array_list<SPtr<DirectionalLightProxy>> m_directionalLights;
        PerRenderObjectDataManager           m_perObjectData;
        float                                m_totalTime = 0.f;
        float                                m_deltaTime = 0.f;
        gfx::GFXBuffer_sp                    m_worldBuffer;
        gfx::GFXBuffer_sp                    m_lightsBuffer;
    };
}
