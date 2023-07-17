#pragma once
#include "AppInstance.h"


namespace pulsar
{
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
        virtual string AppRootDir() override;

        virtual rendering::Pipeline* GetPipeline() override;

        virtual void OnPreInitialize(gfx::GFXGlobalConfig* cfg) override;

        virtual void OnEndRender(float d4) override;

    };

}