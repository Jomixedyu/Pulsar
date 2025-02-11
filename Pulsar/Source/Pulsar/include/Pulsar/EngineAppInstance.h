#pragma once
#include "AppInstance.h"
#include "Components/SceneCaptureComponent.h"

namespace pulsar
{
    class World;

    class EngineRenderPipeline : public gfx::GFXRenderPipeline
    {
    public:
        EngineRenderPipeline(const std::initializer_list<World*>& worlds);

        void RenderImmediate(SceneCaptureComponent* capture);

        void OnRenderCamera(ObjectPtr<CameraComponent> camera);

        void OnRender(gfx::GFXRenderContext* context, gfx::GFXFrameBufferObject* backbuffer) override;

        void AddWorld(World* world);
        void RemoveWorld(World* world);

    protected:
        array_list<World*> m_worlds;
    };


    class EngineAppInstance : public AppInstance
    {
    public:
        virtual const char* AppType() override;
        virtual void OnInitialized() override;
        virtual void OnTerminate() override;
        virtual void OnBeginRender(float dt) override;
        virtual bool IsQuit() override;
        virtual void RequestQuit() override;
        virtual Vector2f GetOutputScreenSize() override;
        virtual void SetOutputScreenSize(Vector2f size) override;
        virtual string GetTitle() override;
        virtual void SetTitle(string_view title) override;
        virtual std::filesystem::path AppRootDir() override;
        virtual AssetManager* GetAssetManager() override;
        virtual rendering::Pipeline* GetPipeline() override;

        virtual void OnPreInitialize(gfx::GFXGlobalConfig* cfg) override;

        virtual void OnEndRender(float d4) override;

    };

}