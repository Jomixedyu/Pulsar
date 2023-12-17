#pragma once
#include <CoreLib/Events.hpp>
#include <Pulsar/ObjectBase.h>
#include <Pulsar/World.h>
#include "Subsystem.h"
#include <Pulsar/Rendering/Pipelines/Pipeline.h>
#include <gfx/GFXApplication.h>


namespace pulsar
{
    class Material;
    class AssetManager;



    class AppInstance
    {
        friend class Application;
    public:
        virtual ~AppInstance() = default;
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

        virtual AssetManager* GetAssetManager() = 0;

        virtual void OnPreInitialize(gfx::GFXGlobalConfig* cfg) = 0;
        virtual void OnInitialized() = 0;
        virtual void OnTerminate() = 0;
        virtual void OnBeginRender(float dt) = 0;
        virtual void OnEndRender(float d4) = 0;
        virtual bool IsQuit() = 0;

        Action<float> OnRenderTick;
    protected:
        array_list<Subsystem_sp> subsystems;
    protected:

    };
}