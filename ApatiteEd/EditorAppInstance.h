#pragma once
#include "Assembly.h"
#include <Apatite/AppInstance.h>

namespace apatiteed
{
    class EditorAppInstance : public AppInstance
    {

    public:
        virtual const char* AppType() override;
        virtual void RequestQuit() override;
        virtual Vector2f ScreenSize() override;
        virtual void SetScreenSize(Vector2f size) override;
        virtual string GetTitle() override;
        virtual void SetTitle(string_view title) override;
        virtual const string& AppRootDir() override;
        virtual void OnInitialize(string_view title, Vector2f size) override;
        virtual void OnTerminate() override;
        virtual void OnTick(float dt) override;
        virtual bool IsQuit() override;
    };
}