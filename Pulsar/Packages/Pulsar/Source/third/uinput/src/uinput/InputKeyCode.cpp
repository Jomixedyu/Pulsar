#include "InputKeyCode.h"
#include <map>
#include <algorithm>
#include <ranges>

namespace uinput
{

    static auto& Map()
    {
        static std::map<KeyCode, std::string> map
        {
            {KeyCode::SPACE, "SPACE"},
            {KeyCode::APOSTROPHE, "APOSTROPHE"},
            {KeyCode::COMMA, "COMMA"},
            {KeyCode::MINUS, "MINUS"},
            {KeyCode::PERIOD, "PERIOD"},
            {KeyCode::SLASH, "SLASH"},
            {KeyCode::NUM_0, "NUM_0"},
            {KeyCode::NUM_1, "NUM_1"},
            {KeyCode::NUM_2, "NUM_2"},
            {KeyCode::NUM_3, "NUM_3"},
            {KeyCode::NUM_4, "NUM_4"},
            {KeyCode::NUM_5, "NUM_5"},
            {KeyCode::NUM_6, "NUM_6"},
            {KeyCode::NUM_7, "NUM_7"},
            {KeyCode::NUM_8, "NUM_8"},
            {KeyCode::NUM_9, "NUM_9"},
            {KeyCode::SEMICOLON, "SEMICOLON"},
            {KeyCode::EQUAL, "EQUAL"},
            {KeyCode::A, "A"},
            {KeyCode::B, "B"},
            {KeyCode::C, "C"},
            {KeyCode::D, "D"},
            {KeyCode::E, "E"},
            {KeyCode::F, "F"},
            {KeyCode::G, "G"},
            {KeyCode::H, "H"},
            {KeyCode::I, "I"},
            {KeyCode::J, "J"},
            {KeyCode::K, "K"},
            {KeyCode::L, "L"},
            {KeyCode::M, "M"},
            {KeyCode::N, "N"},
            {KeyCode::O, "O"},
            {KeyCode::P, "P"},
            {KeyCode::Q, "Q"},
            {KeyCode::R, "R"},
            {KeyCode::S, "S"},
            {KeyCode::T, "T"},
            {KeyCode::U, "U"},
            {KeyCode::V, "V"},
            {KeyCode::W, "W"},
            {KeyCode::X, "X"},
            {KeyCode::Y, "Y"},
            {KeyCode::Z, "Z"},
            {KeyCode::LEFT_BRACKET, "LEFT_BRACKET"},
            {KeyCode::BACKSLASH, "BACKSLASH"},
            {KeyCode::RIGHT_BRACKET, "RIGHT_BRACKET"},
            {KeyCode::GRAVE_ACCENT, "GRAVE_ACCENT"},
            {KeyCode::ESCAPE, "ESCAPE"},
            {KeyCode::ENTER, "ENTER"},
            {KeyCode::TAB, "TAB"},
            {KeyCode::BACKSPACE, "BACKSPACE"},
            {KeyCode::INSERT, "INSERT"},
            {KeyCode::DELETE, "DELETE"},
            {KeyCode::RIGHT, "RIGHT"},
            {KeyCode::LEFT, "LEFT"},
            {KeyCode::DOWN, "DOWN"},
            {KeyCode::UP, "UP"},
            {KeyCode::PAGE_UP, "PAGE_UP"},
            {KeyCode::PAGE_DOWN, "PAGE_DOWN"},
            {KeyCode::HOME, "HOME"},
            {KeyCode::END, "END"},
            {KeyCode::CAPS_LOCK, "CAPS_LOCK"},
            {KeyCode::SCROLL_LOCK, "SCROLL_LOCK"},
            {KeyCode::NUM_LOCK, "NUM_LOCK"},
            {KeyCode::PRINT_SCREEN, "PRINT_SCREEN"},
            {KeyCode::PAUSE, "PAUSE"},
            {KeyCode::F1, "F1"},
            {KeyCode::F2, "F2"},
            {KeyCode::F3, "F3"},
            {KeyCode::F4, "F4"},
            {KeyCode::F5, "F5"},
            {KeyCode::F6, "F6"},
            {KeyCode::F7, "F7"},
            {KeyCode::F8, "F8"},
            {KeyCode::F9, "F9"},
            {KeyCode::F10, "F10"},
            {KeyCode::F11, "F11"},
            {KeyCode::F12, "F12"},
            {KeyCode::F13, "F13"},
            {KeyCode::F14, "F14"},
            {KeyCode::F15, "F15"},
            {KeyCode::F16, "F16"},
            {KeyCode::F17, "F17"},
            {KeyCode::F18, "F18"},
            {KeyCode::F19, "F19"},
            {KeyCode::F20, "F20"},
            {KeyCode::F21, "F21"},
            {KeyCode::F22, "F22"},
            {KeyCode::F23, "F23"},
            {KeyCode::F24, "F24"},
            {KeyCode::KP_0, "KP_0"},
            {KeyCode::KP_1, "KP_1"},
            {KeyCode::KP_2, "KP_2"},
            {KeyCode::KP_3, "KP_3"},
            {KeyCode::KP_4, "KP_4"},
            {KeyCode::KP_5, "KP_5"},
            {KeyCode::KP_6, "KP_6"},
            {KeyCode::KP_7, "KP_7"},
            {KeyCode::KP_8, "KP_8"},
            {KeyCode::KP_9, "KP_9"},
            {KeyCode::KP_DECIMAL, "KP_DECIMAL"},
            {KeyCode::KP_DIVIDE, "KP_DIVIDE"},
            {KeyCode::KP_MULTIPLY, "KP_MULTIPLY"},
            {KeyCode::KP_SUBTRACT, "KP_SUBTRACT"},
            {KeyCode::KP_ADD, "KP_ADD"},
            {KeyCode::KP_ENTER, "KP_ENTER"},
            {KeyCode::KP_EQUAL, "KP_EQUAL"},
            {KeyCode::LEFT_SHIFT, "LEFT_SHIFT"},
            {KeyCode::LEFT_CONTROL, "LEFT_CONTROL"},
            {KeyCode::LEFT_ALT, "LEFT_ALT"},
            {KeyCode::RIGHT_SHIFT, "RIGHT_SHIFT"},
            {KeyCode::RIGHT_CONTROL, "RIGHT_CONTROL"},
            {KeyCode::RIGHT_ALT, "RIGHT_ALT"},
            {KeyCode::MENU, "MENU"},
        };
        return map;
    }
    static auto& RevMap()
    {
        static auto map =
            std::views::transform(Map(),
                [](const std::pair<KeyCode, std::string>& pair){ return std::pair(pair.second, pair.first); })
                | std::ranges::to<std::map<std::string, KeyCode>>();
        return map;
    }

    KeyCode KeyCodeParse(const char* name)
    {
        return RevMap()[name];
    }

    std::string to_string(KeyCode code)
    {
        return Map()[code];
    }

    std::vector<std::pair<std::string, uint32_t>> BuildKeyCodeMap()
    {
        std::vector<std::pair<std::string, uint32_t>> map;
        for (auto& pair : Map())
        {
            map.emplace_back(pair.second, static_cast<uint32_t>(pair.first));
        }
        return map;
    }

} // namespace uinput