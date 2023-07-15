#pragma once

#include <CoreLib/Events.hpp>
#include <Pulsar/ObjectBase.h>
#include "AppInstance.h"
#include "Math.h"
#include <gfx/GFXApplication.h>

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

    };

}