#include <Pulsar/Application.h>
#include <Pulsar/Logger.h>
#include <gfx-vk/GFXVulkanApplication.h>
#include "AppInstance.h"
#include <chrono>
#include <cstdlib>


namespace pulsar
{
    namespace
    {
        struct AutoQuitState
        {
            int DelayMs = 0;
            bool Enabled = false;
            bool Triggered = false;
            std::chrono::steady_clock::time_point StartTime{};
        };
    }

    static AppInstance* g_currentInst;
    static int g_argc = 0;
    static const char* const* g_argv = nullptr;

    AppInstance* Application::inst()
    {
        return g_currentInst;
    }
    static gfx::GFXVulkanApplication* g_gfxApp = nullptr;
    gfx::GFXApplication* Application::GetGfxApp()
    {
        return g_gfxApp;
    }

    int Application::argc()
    {
        return g_argc;
    }

    const char* const* Application::argv()
    {
        return g_argv;
    }

    bool Application::TryGetCommandLineArg(string_view prefix, string* outValue)
    {
        if (!g_argv || prefix.empty())
        {
            return false;
        }

        for (int i = 1; i < g_argc; ++i)
        {
            string_view arg = g_argv[i];
            if (arg.starts_with(prefix))
            {
                if (outValue)
                {
                    *outValue = string(arg.substr(prefix.size()));
                }
                return true;
            }
        }
        return false;
    }

    int Application::Exec(AppInstance* instance, string_view title, Vector2f size, int argc, const char* const* argv)
    {
        Watch.Start();

        g_currentInst = instance;
        g_argc = argc;
        g_argv = argv;

        gfx::GFXGlobalConfig gfxConfig{};
        instance->OnPreInitialize(&gfxConfig);

        if (gfxConfig.WindowWidth < 0 || gfxConfig.WindowHeight < 0)
        {
            assert(false);
        }

        Watch.Record("preinitiialize");

        g_gfxApp = new gfx::GFXVulkanApplication(gfxConfig);
        g_gfxApp->Initialize();

        Watch.Record("gfx initialize");

        instance->OnInitialized();

        Watch.Record("user initialize");

        AutoQuitState autoQuit;
        autoQuit.StartTime = std::chrono::steady_clock::now();
        string autoQuitValue;
        if (TryGetCommandLineArg("--auto-quit-ms=", &autoQuitValue))
        {
            autoQuit.DelayMs = std::atoi(autoQuitValue.c_str());
            autoQuit.Enabled = autoQuit.DelayMs > 0;
            if (autoQuit.Enabled)
            {
                Watch.Record("auto quit armed");
                Logger::Log("Application auto quit enabled: " + std::to_string(autoQuit.DelayMs) + " ms", LogLevel::Warning);
            }
        }

        g_gfxApp->OnPreRender = [autoQuit](float dt) mutable
        {
            const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                std::chrono::steady_clock::now() - autoQuit.StartTime).count();
            if (autoQuit.Enabled && !autoQuit.Triggered && elapsedMs >= autoQuit.DelayMs)
            {
                autoQuit.Triggered = true;
                Logger::Log("Application auto quit triggered", LogLevel::Warning);
                g_currentInst->RequestQuit();
                g_gfxApp->RequestStop();
                return;
            }
            g_currentInst->OnBeginRender(dt);
            if (g_currentInst->IsQuit())
            {
                g_gfxApp->RequestStop();
            }
        };
        g_gfxApp->OnPostRender = [](float dt)
        {
            g_currentInst->OnEndRender(dt);
            if (g_currentInst->IsQuit())
            {
                g_gfxApp->RequestStop();
            }
        };
        g_gfxApp->ExecLoop();
        instance->OnTerminate();
        g_gfxApp->Terminate();
        delete g_gfxApp;
        g_gfxApp = nullptr;
        g_currentInst = nullptr;
        g_argc = 0;
        g_argv = nullptr;

        return 0;
    }

}


