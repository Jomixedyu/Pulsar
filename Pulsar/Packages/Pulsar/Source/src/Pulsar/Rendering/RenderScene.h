#pragma once
#include "PerRenderObjectDataManager.h"
#include "PerPassData.h"
#include "RenderObject.h"
#include "RenderProxy.h"
#include "LightProxy.h"
#include "SceneView.h"
#include <gfx/GFXBuffer.h>
#include <memory>

namespace pulsar
{
    class SceneRenderPipeline;

    class RenderScene
    {
    public:
        RenderScene();
        ~RenderScene();

        RenderScene(const RenderScene&) = delete;
        RenderScene& operator=(const RenderScene&) = delete;

        void AddProxy(SPtr<rendering::RenderProxy> proxy);
        void RemoveProxy(const SPtr<rendering::RenderProxy>& proxy);
        void Destroy();

        const hash_set<rendering::RenderObject_sp>& GetRenderObjects() const { return m_renderObjects; }
        const array_list<SPtr<SceneView>>& GetViews() const { return m_views; }
        const array_list<SPtr<PointLightProxy>>& GetPointLights() const { return m_pointLights; }
        const array_list<SPtr<DirectionalLightProxy>>& GetDirectionalLights() const { return m_directionalLights; }
        PerRenderObjectDataManager& GetPerRenderObjectData() { return m_perObjectData; }
        SceneRenderPipeline& GetRenderPipeline();

        void SetTime(float totalTime, float deltaTime) { m_totalTime = totalTime; m_deltaTime = deltaTime; }
        float GetTotalTime() const { return m_totalTime; }
        float GetDeltaTime() const { return m_deltaTime; }

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
        array_list<SPtr<SceneView>> m_views;
        array_list<SPtr<PointLightProxy>> m_pointLights;
        array_list<SPtr<DirectionalLightProxy>> m_directionalLights;
        std::unique_ptr<SceneRenderPipeline> m_renderPipeline;
        PerRenderObjectDataManager m_perObjectData;
        float m_totalTime = 0.f;
        float m_deltaTime = 0.f;
        gfx::GFXBuffer_sp m_worldBuffer;
        gfx::GFXBuffer_sp m_lightsBuffer;
    };
}
