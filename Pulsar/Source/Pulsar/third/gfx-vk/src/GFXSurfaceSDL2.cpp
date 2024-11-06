#include "GFXSurfaceSDL2.h"

#include <SDL_syswm.h>
#include <SDL_vulkan.h>
#include <format>
#include <stdexcept>

#undef CreateWindow

namespace gfx
{

    void GFXSurfaceSDL2::Initialize()
    {
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER | SDL_INIT_GAMECONTROLLER) != 0)
        {
            throw std::runtime_error(SDL_GetError());
        }
    }

    void GFXSurfaceSDL2::CreateMainWindow()
    {
        SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
        m_window = SDL_CreateWindow("x", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 1280, 720, window_flags);
        if (m_window == nullptr)
        {
            auto info = std::format("Error: SDL_CreateWindow(): {}", SDL_GetError());
            throw std::runtime_error(info);
        }
    }
    int GFXSurfaceSDL2::CreateVulkanSurface(void* instance, void* outSurface)
    {
        return SDL_Vulkan_CreateSurface(m_window, (VkInstance)instance, (VkSurfaceKHR*)outSurface);
    }
    void* GFXSurfaceSDL2::GetUserPoint() const
    {
        return m_window;
    }

    intptr_t GFXSurfaceSDL2::GetNativeHandle() const
    {
        SDL_SysWMinfo info;
        SDL_GetWindowWMInfo(m_window, &info);
        return reinterpret_cast<intptr_t>(info.info.win.window);
    }

    void GFXSurfaceSDL2::DestroySurface()
    {
        SDL_DestroyWindow(m_window);
        SDL_Quit();
    }
    bool GFXSurfaceSDL2::WantToClose()
    {
        return m_wantToClose;
    }
    void GFXSurfaceSDL2::PollEvent()
    {
        m_eventCount = 0;

        constexpr auto max = sizeof(m_events) / sizeof(SDL_Event);

        SDL_Event* curEvent = &m_events[m_eventCount];

        m_wantToClose = false;
        while (SDL_PollEvent(curEvent) && m_eventCount != max)
        {
            auto& event = *curEvent;
            m_wantToClose |= event.type == SDL_QUIT;
            m_wantToClose |= event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(m_window);
            ++m_eventCount;
            curEvent = &m_events[m_eventCount];
        }
    }
    void GFXSurfaceSDL2::EnumEvents(void (*func)(void*))
    {
        if (func == nullptr)
        {
            return;
        }
        for (int i = 0; i < m_eventCount; ++i)
        {
            func(&m_events[i]);
        }
    }

} // namespace gfx