#pragma once
#include "AppInstance.h"

namespace apatite
{
    class EngineAppInstance : public AppInstance
    {
    public:
        virtual const char* AppType() override;
        virtual void OnInitialize(string_view title, Vector2f size) override;
        virtual void OnTerminate() override;
        virtual void OnTick(float dt) override;
        virtual bool IsQuit() override;
        virtual void RequestQuit() override;
        virtual Vector2f ScreenSize() override;
        virtual void SetScreenSize(Vector2f size) override;
        virtual string GetTitle() override;
        virtual void SetTitle(string_view title) override;
        virtual string AppRootDir() override;
    };

}