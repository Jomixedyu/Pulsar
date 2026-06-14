#include <Pulsar/Application.h>
#include <Pulsar/Logger.h>
#include <gfx-vk/GFXVulkanApplication.h>
#include "AppInstance.h"
#include "Rendering/RenderThread.h"
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

    static RenderThread* g_renderThread = nullptr;
    RenderThread* Application::GetRenderThread()
    {
        return g_renderThread;
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

        // Initialize file logging before anything else so all startup logs are captured
        {
            auto logDir = instance->GetTempDirectory() / "Logs";
            std::filesystem::create_directories(logDir);

            time_t now = std::time(nullptr);
            struct tm* local = std::localtime(&now);
            char filename[128];
            ::sprintf_s(filename, 128, "Pulsar_%04d%02d%02d_%02d%02d%02d.log",
                        local->tm_year + 1900, local->tm_mon + 1, local->tm_mday,
                        local->tm_hour, local->tm_min, local->tm_sec);
            auto logPath = logDir / filename;
            Logger::InitializeFileLogging(logPath.string());
            Logger::Log("File logging initialized: " + logPath.string());
        }

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

        g_renderThread = new RenderThread();
        g_renderThread->Start_AnyThread();

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

        // ========== Engine-owned frame loop (lockstep) ==========
        // Main thread: poll window/input -> tick game -> dispatch render to the
        // render thread and block until it has drained pending resource updates
        // and drawn the frame. Rendering never overlaps the game tick.
        gfx::GFXSurface* window = g_gfxApp->GetWindow();
        auto lastTime = std::chrono::steady_clock::now();
        while (true)
        {
            auto currentTime = std::chrono::steady_clock::now();
            float dt = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - lastTime).count();
            lastTime = currentTime;

            if (window->WantToClose())
            {
                g_currentInst->RequestQuit();
            }
            window->PollEvent();

            if (autoQuit.Enabled && !autoQuit.Triggered)
            {
                const auto elapsedMs = std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::steady_clock::now() - autoQuit.StartTime).count();
                if (elapsedMs >= autoQuit.DelayMs)
                {
                    autoQuit.Triggered = true;
                    Logger::Log("Application auto quit triggered", LogLevel::Warning);
                    g_currentInst->RequestQuit();
                }
            }

            // game tick
            g_currentInst->OnBeginRender(dt);
            if (g_currentInst->IsQuit())
            {
                break;
            }

            // render on the render thread, block until done
            g_renderThread->RunFrame_AnyThread([dt] { g_gfxApp->TickRender(dt); });

            g_currentInst->OnEndRender(dt);
            if (g_currentInst->IsQuit())
            {
                break;
            }
        }

        instance->OnTerminate();

        // World::OnWorldEnd already drained its own teardown synchronously. This is a
        // backstop: any destroys enqueued by the rest of OnTerminate (e.g. shader cache
        // clear / transient RT pool shutdown) run here, after vkDeviceWaitIdle, while the
        // GFX device + resource manager are still alive, before we stop the thread.
        g_renderThread->WaitForIdle_AnyThread();

        g_renderThread->Stop_AnyThread();
        delete g_renderThread;
        g_renderThread = nullptr;

        g_gfxApp->Terminate();
        delete g_gfxApp;
        g_gfxApp = nullptr;
        g_currentInst = nullptr;
        g_argc = 0;
        g_argv = nullptr;

        Logger::ShutdownFileLogging();

        return 0;
    }

}


