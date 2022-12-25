#pragma once
#include <CoreLib/Events.hpp>
#include <Apatite/Math.h>
#include <Apatite/ObjectBase.h>

namespace apatite
{
    class AppInstance
    {
        friend class Application;
    public:
        virtual const char* AppType() = 0;
        virtual void RequestQuit() = 0;
        virtual Vector2f ScreenSize() = 0;
        virtual void SetScreenSize(Vector2f size) = 0;
        virtual string GetTitle() = 0;
        virtual void SetTitle(string_view title) = 0;

        virtual string AppRootDir() = 0;
        jxcorlib::Action<> QuittingEvents;
        jxcorlib::Function<bool> RequestQuitEvents;

    protected:
        virtual void OnInitialize(string_view title, Vector2f size) = 0;
        virtual void OnTerminate() = 0;
        virtual void OnTick(float dt) = 0;
        virtual bool IsQuit() = 0;
    };
}