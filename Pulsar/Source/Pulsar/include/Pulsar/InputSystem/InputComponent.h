#pragma once
#include "Components/Component.h"
#include "InputValueType.h"
#include "InputActionMap.h"
#include <set>

namespace pulsar
{

    class InputContext : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputContext, Object);
    public:
        virtual float GetFloatValue() const { return m_value.m_value.x; }
        virtual Vector2f GetVector2() const { return {m_value.m_value.x, m_value.m_value.y}; }
        virtual Vector3f GetVector3() const { return m_value.m_value; }

        InputActionValue m_value;
    };
    CORELIB_DECL_SHORTSPTR(InputContext);


    struct InputKeyboardState
    {
        std::set<KeyCode> m_downKey;

        bool IsKeyDown(KeyCode code) const { return m_downKey.contains(code); }
    };

    struct InputEventState
    {
        InputActionBinding_sp binding;
        string slot;
        InputValueType type;
    };

    class InputComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputComponent, Component);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Input"));
    public:
        using InputEventDelegate = FunctionDelegate<void, SPtr<InputContext>>;
        virtual void Bind(string_view name, SPtr<InputEventDelegate> callback);

        void OnTick(Ticker ticker) override;
        void BeginPlay() override;
        void EndPlay() override;

        void RegisterInputMap();



        CORELIB_REFL_DECL_FIELD(m_actionMap);
        RCPtr<InputActionMap> m_actionMap;

        hash_map<string, array_list<SPtr<InputEventDelegate>>> m_eventMap;

        InputKeyboardState m_keyboard;

        int m_keyboardCallbackHandle{};
    };
    DECL_PTR(InputComponent)
} // namespace pulsar