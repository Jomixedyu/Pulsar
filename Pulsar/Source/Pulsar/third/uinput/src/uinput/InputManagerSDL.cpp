#include "InputManagerSDL.h"
#include <optional>

namespace uinput
{
    static std::optional<KeyCode> SDLKeyCode2KeyCode(SDL_Keycode code)
    {
        static std::unordered_map<SDL_Keycode, KeyCode> map =
#pragma region codes
            {
                {SDL_KeyCode::SDLK_SPACE, KeyCode::SPACE},
                {SDL_KeyCode::SDLK_QUOTE, KeyCode::APOSTROPHE}, /* ' */
                {SDL_KeyCode::SDLK_COMMA, KeyCode::COMMA},      /* , */
                {SDL_KeyCode::SDLK_MINUS, KeyCode::MINUS},      /* - */
                {SDL_KeyCode::SDLK_PERIOD, KeyCode::PERIOD},     /* . */
                {SDL_KeyCode::SDLK_SLASH, KeyCode::SLASH},      /* / */
                {SDL_KeyCode::SDLK_0, KeyCode::NUM_0},
                {SDL_KeyCode::SDLK_1, KeyCode::NUM_1},
                {SDL_KeyCode::SDLK_2, KeyCode::NUM_2},
                {SDL_KeyCode::SDLK_3, KeyCode::NUM_3},
                {SDL_KeyCode::SDLK_4, KeyCode::NUM_4},
                {SDL_KeyCode::SDLK_5, KeyCode::NUM_5},
                {SDL_KeyCode::SDLK_6, KeyCode::NUM_6},
                {SDL_KeyCode::SDLK_7, KeyCode::NUM_7},
                {SDL_KeyCode::SDLK_8, KeyCode::NUM_8},
                {SDL_KeyCode::SDLK_9, KeyCode::NUM_9},
                {SDL_KeyCode::SDLK_SEMICOLON, KeyCode::SEMICOLON}, /* ; */
                {SDL_KeyCode::SDLK_EQUALS, KeyCode::EQUAL},     /* = */
                {SDL_KeyCode::SDLK_a, KeyCode::A},
                {SDL_KeyCode::SDLK_b, KeyCode::B},
                {SDL_KeyCode::SDLK_c, KeyCode::C},
                {SDL_KeyCode::SDLK_d, KeyCode::D},
                {SDL_KeyCode::SDLK_e, KeyCode::E},
                {SDL_KeyCode::SDLK_f, KeyCode::F},
                {SDL_KeyCode::SDLK_g, KeyCode::G},
                {SDL_KeyCode::SDLK_h, KeyCode::H},
                {SDL_KeyCode::SDLK_i, KeyCode::I},
                {SDL_KeyCode::SDLK_j, KeyCode::J},
                {SDL_KeyCode::SDLK_k, KeyCode::K},
                {SDL_KeyCode::SDLK_l, KeyCode::L},
                {SDL_KeyCode::SDLK_m, KeyCode::M},
                {SDL_KeyCode::SDLK_n, KeyCode::N},
                {SDL_KeyCode::SDLK_o, KeyCode::O},
                {SDL_KeyCode::SDLK_p, KeyCode::P},
                {SDL_KeyCode::SDLK_q, KeyCode::Q},
                {SDL_KeyCode::SDLK_r, KeyCode::R},
                {SDL_KeyCode::SDLK_s, KeyCode::S},
                {SDL_KeyCode::SDLK_t, KeyCode::T},
                {SDL_KeyCode::SDLK_u, KeyCode::U},
                {SDL_KeyCode::SDLK_v, KeyCode::V},
                {SDL_KeyCode::SDLK_w, KeyCode::W},
                {SDL_KeyCode::SDLK_x, KeyCode::X},
                {SDL_KeyCode::SDLK_y, KeyCode::Y},
                {SDL_KeyCode::SDLK_z, KeyCode::Z},
                {SDL_KeyCode::SDLK_LEFTBRACKET, KeyCode::LEFT_BRACKET},  /* [ */
                {SDL_KeyCode::SDLK_BACKSLASH, KeyCode::BACKSLASH},     /* \ */
                {SDL_KeyCode::SDLK_RIGHTBRACKET, KeyCode::RIGHT_BRACKET}, /* ] */
                {SDL_KeyCode::SDLK_BACKQUOTE, KeyCode::GRAVE_ACCENT},  /* ` */
                // {SDL_KeyCode::SDLK_0, KeyCode::WORLD_1},       /* non-US #1 */
                // {SDL_KeyCode::SDLK_0, KeyCode::WORLD_2},       /* non-US #2 */
                {SDL_KeyCode::SDLK_ESCAPE, KeyCode::ESCAPE},
                {SDL_KeyCode::SDLK_RETURN, KeyCode::ENTER},
                {SDL_KeyCode::SDLK_TAB, KeyCode::TAB},
                {SDL_KeyCode::SDLK_BACKSPACE, KeyCode::BACKSPACE},
                {SDL_KeyCode::SDLK_INSERT, KeyCode::INSERT},
                {SDL_KeyCode::SDLK_DELETE, KeyCode::DELETE},
                {SDL_KeyCode::SDLK_RIGHT, KeyCode::RIGHT},
                {SDL_KeyCode::SDLK_LEFT, KeyCode::LEFT},
                {SDL_KeyCode::SDLK_DOWN, KeyCode::DOWN},
                {SDL_KeyCode::SDLK_UP, KeyCode::UP},
                {SDL_KeyCode::SDLK_PAGEUP, KeyCode::PAGE_UP},
                {SDL_KeyCode::SDLK_PAGEDOWN, KeyCode::PAGE_DOWN},
                {SDL_KeyCode::SDLK_HOME, KeyCode::HOME},
                {SDL_KeyCode::SDLK_END, KeyCode::END},
                {SDL_KeyCode::SDLK_CAPSLOCK, KeyCode::CAPS_LOCK},
                {SDL_KeyCode::SDLK_SCROLLLOCK, KeyCode::SCROLL_LOCK},
                {SDL_KeyCode::SDLK_NUMLOCKCLEAR, KeyCode::NUM_LOCK},
                {SDL_KeyCode::SDLK_PRINTSCREEN, KeyCode::PRINT_SCREEN},
                {SDL_KeyCode::SDLK_PAUSE, KeyCode::PAUSE},
                {SDL_KeyCode::SDLK_F1, KeyCode::F1},
                {SDL_KeyCode::SDLK_F2, KeyCode::F2},
                {SDL_KeyCode::SDLK_F3, KeyCode::F3},
                {SDL_KeyCode::SDLK_F4, KeyCode::F4},
                {SDL_KeyCode::SDLK_F5, KeyCode::F5},
                {SDL_KeyCode::SDLK_F6, KeyCode::F6},
                {SDL_KeyCode::SDLK_F7, KeyCode::F7},
                {SDL_KeyCode::SDLK_F8, KeyCode::F8},
                {SDL_KeyCode::SDLK_F9, KeyCode::F9},
                {SDL_KeyCode::SDLK_F10, KeyCode::F10},
                {SDL_KeyCode::SDLK_F11, KeyCode::F11},
                {SDL_KeyCode::SDLK_F12, KeyCode::F12},
                {SDL_KeyCode::SDLK_F13, KeyCode::F13},
                {SDL_KeyCode::SDLK_F14, KeyCode::F14},
                {SDL_KeyCode::SDLK_F15, KeyCode::F15},
                {SDL_KeyCode::SDLK_F16, KeyCode::F16},
                {SDL_KeyCode::SDLK_F17, KeyCode::F17},
                {SDL_KeyCode::SDLK_F18, KeyCode::F18},
                {SDL_KeyCode::SDLK_F19, KeyCode::F19},
                {SDL_KeyCode::SDLK_F20, KeyCode::F20},
                {SDL_KeyCode::SDLK_F21, KeyCode::F21},
                {SDL_KeyCode::SDLK_F22, KeyCode::F22},
                {SDL_KeyCode::SDLK_F23, KeyCode::F23},
                {SDL_KeyCode::SDLK_F24, KeyCode::F24},
                // {SDL_KeyCode::SDLK_F25, KeyCode::F25},
                {SDL_KeyCode::SDLK_KP_0, KeyCode::KP_0},
                {SDL_KeyCode::SDLK_KP_1, KeyCode::KP_1},
                {SDL_KeyCode::SDLK_KP_2, KeyCode::KP_2},
                {SDL_KeyCode::SDLK_KP_3, KeyCode::KP_3},
                {SDL_KeyCode::SDLK_KP_4, KeyCode::KP_4},
                {SDL_KeyCode::SDLK_KP_5, KeyCode::KP_5},
                {SDL_KeyCode::SDLK_KP_6, KeyCode::KP_6},
                {SDL_KeyCode::SDLK_KP_7, KeyCode::KP_7},
                {SDL_KeyCode::SDLK_KP_8, KeyCode::KP_8},
                {SDL_KeyCode::SDLK_KP_9, KeyCode::KP_9},
                {SDL_KeyCode::SDLK_KP_DECIMAL, KeyCode::KP_DECIMAL},
                {SDL_KeyCode::SDLK_KP_DIVIDE, KeyCode::KP_DIVIDE},
                {SDL_KeyCode::SDLK_KP_MULTIPLY, KeyCode::KP_MULTIPLY},
                {SDL_KeyCode::SDLK_KP_MINUS, KeyCode::KP_SUBTRACT},
                {SDL_KeyCode::SDLK_KP_PLUS, KeyCode::KP_ADD},
                {SDL_KeyCode::SDLK_KP_ENTER, KeyCode::KP_ENTER},
                {SDL_KeyCode::SDLK_KP_EQUALS, KeyCode::KP_EQUAL},
                {SDL_KeyCode::SDLK_LSHIFT, KeyCode::LEFT_SHIFT},
                {SDL_KeyCode::SDLK_LCTRL, KeyCode::LEFT_CONTROL},
                {SDL_KeyCode::SDLK_LALT, KeyCode::LEFT_ALT},
                // {SDL_KeyCode::LSu, KeyCode::LEFT_SUPER},
                {SDL_KeyCode::SDLK_RSHIFT, KeyCode::RIGHT_SHIFT},
                {SDL_KeyCode::SDLK_RCTRL, KeyCode::RIGHT_CONTROL},
                {SDL_KeyCode::SDLK_RALT, KeyCode::RIGHT_ALT},
                // {SDL_KeyCode::SDLK_0, KeyCode::RIGHT_SUPER},
                {SDL_KeyCode::SDLK_MENU, KeyCode::MENU},
            };
#pragma endregion codes
        auto it = map.find(code);
        if (it != map.end())
        {
            return it->second;
        }
        return {};
    }

    static int OnEvent(void* user, SDL_Event* event)
    {
        auto* self = (InputManagerSDL*)user;
        switch (event->type)
        {
        case SDL_KEYDOWN:
        case SDL_KEYUP:
        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        case SDL_MOUSEMOTION:
        case SDL_MOUSEWHEEL:
            self->PushEvent(event);
        default:;
        }

        return 0;
    }

    void InputManagerSDL::Initialize()
    {
        InputManager::Initialize();
        SDL_AddEventWatch(OnEvent, this);
    }

    void InputManagerSDL::ProcessEvents()
    {
        InputManager::ProcessEvents();

        for (auto& e : m_pendingEvents)
        {
            switch (e.type)
            {
            case SDL_KEYDOWN: {
                if (auto value = SDLKeyCode2KeyCode(e.key.keysym.sym))
                {
                    BroadcastKeyboard(KeyState::Down, value.value());
                }
                break;
            }
            case SDL_KEYUP: {
                if (auto value = SDLKeyCode2KeyCode(e.key.keysym.sym))
                {
                    BroadcastKeyboard(KeyState::Up, value.value());
                }
                break;
            }
            default:;
            }
        }

        m_pendingEvents.clear();
    }
    void InputManagerSDL::PushEvent(SDL_Event* event)
    {
        m_mutex.lock();
        m_pendingEvents.push_back(*event);
        m_mutex.unlock();
    }

} // namespace uinput