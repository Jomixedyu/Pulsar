#pragma once
#include "InputManager.h"
#include <SDL.h>
#include <vector>
#include <mutex>

namespace uinput
{
    class InputManagerSDL : public InputManager
    {
    public:
        void Initialize() override;
        void ProcessEvents() override;
        void PushEvent(SDL_Event* event);
    private:
        std::vector<SDL_Event> m_pendingEvents;
        std::mutex m_mutex;
    };
} // namespace uinput