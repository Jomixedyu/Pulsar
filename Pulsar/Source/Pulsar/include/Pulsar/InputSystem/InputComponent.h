#pragma once
#include "Components/Component.h"
#include "InputValueType.h"
#include "InputActionMap.h"


namespace pulsar
{

    class InputContext : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputContext, Object);
    public:
        virtual float GetFloatValue() = 0;

        string_view m_name;
    };
    CORELIB_DECL_SHORTSPTR(InputContext);

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


        CORELIB_REFL_DECL_FIELD(m_actionMap);
        RCPtr<InputActionMap> m_actionMap;

        array_list<SPtr<InputEventDelegate>> m_callbacks;
    };
    DECL_PTR(InputComponent)
} // namespace pulsar