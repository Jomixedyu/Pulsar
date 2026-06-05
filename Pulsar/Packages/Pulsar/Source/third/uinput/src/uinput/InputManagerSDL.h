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
        void Terminate() override;

        std::vector<InputEvent> PollEvents() override;

        InputVector2f GetMousePosition() const override;
        bool IsMouseButtonDown(int button) const override;

    public:
        void PushEvent(const InputEvent& event);

    private:
        std::vector<InputEvent> m_pendingEvents;
        std::mutex m_mutex;
    };
} // namespace uinput
