#pragma once
#include "../../gfx/include/gfx/GFXSurface.h"
#include <SDL.h>

namespace gfx
{
    class GFXSurfaceSDL2 : public GFXSurface
    {
    public:

        GFXSurfaceSDL2() = default;

        void Initialize() override;
        void CreateMainWindow() override;
        int CreateVulkanSurface(void* instance, void* outSurface) override;
        void* GetUserPoint() const override;
        intptr_t GetNativeHandle() const override;
        void DestroySurface() override;

        bool WantToClose() override;
        void PollEvent() override;
        void EnumEvents(void(* func)(void*)) override;

        SDL_Window* m_window = nullptr;
        bool m_wantToClose = false;
        SDL_Event m_events[32]{};
        int m_eventCount = 0;
    };
}
