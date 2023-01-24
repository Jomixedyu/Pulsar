#pragma once
#include <CoreLib/Events.hpp>
#include <Apatite/Math.h>
#include <Apatite/ObjectBase.h>
#include <Apatite/World.h>
#include "Subsystem.h"
#include <Apatite/Rendering/Pipelines/Pipeline.h>

namespace apatite
{
    class AppInstance
    {
        friend class Application;
    public:
        virtual const char* AppType() = 0;
        virtual void RequestQuit() = 0;
        virtual Vector2f GetOutputScreenSize() = 0;
        virtual void SetOutputScreenSize(Vector2f size) = 0;
        virtual string GetTitle() = 0;
        virtual void SetTitle(string_view title) = 0;
        virtual string AppRootDir() = 0;
        virtual rendering::Pipeline* GetPipeline() = 0;

        jxcorlib::Action<> QuittingEvents;
        jxcorlib::Function<bool> RequestQuitEvents;

        virtual Subsystem* GetSubsystemByType(Type* type);
        virtual array_list<Subsystem*> GetSubsystemsByType(Type* type, bool include = true);

        template<typename T>
        sptr<T> GetSubsystem()
        {
            return sptr_cast<T>(GetSubsystemByType(cltypeof<T>()));
        }

    protected:
        array_list<Subsystem_sp> subsystems;
    protected:
        virtual void OnInitialize(string_view title, Vector2f size) = 0;
        virtual void OnTerminate() = 0;
        virtual void OnTick(float dt) = 0;
        virtual bool IsQuit() = 0;
    };
}