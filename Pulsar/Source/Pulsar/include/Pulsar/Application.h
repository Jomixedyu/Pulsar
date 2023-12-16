#pragma once

#include <Pulsar/AppInstance.h>
#include <CoreLib/Events.hpp>
#include <gfx/GFXApplication.h>
#include <CoreLib/StopWatch.h>

namespace pulsar
{
    class Application
    {
    private:
        Application() = delete;
    public:
        static AppInstance* inst();
        static gfx::GFXApplication* GetGfxApp();
        //start
        static int Exec(AppInstance* instance, string_view title, Vector2f size);
        
        static inline jxcorlib::StopWatch Watch {"syswatch"};

    };

    class AppInstanceTickable
    {
    private:
        uint32_t m_handler;
    public:
        AppInstanceTickable()
        {
            m_handler = Application::inst()->OnRenderTick.AddListener(this, &AppInstanceTickable::Tick);
        }
        virtual void Tick(float dt) = 0;
        virtual ~AppInstanceTickable()
        {
            Application::inst()->OnRenderTick.RemoveListenerByIndex(m_handler);
        }
    };

}