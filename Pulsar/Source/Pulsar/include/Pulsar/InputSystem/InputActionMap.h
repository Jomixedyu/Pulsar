#pragma once
#include "InputAction.h"
#include "Pulsar/AssetObject.h"
#include "KeyCode.h"
#include "InputModifier.h"

namespace pulsar
{


    class InputActionBinding : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputActionBinding, Object);
    public:
        InputActionBinding()
        {
            init_sptr_member(m_negative);
            init_sptr_member(m_sizzleMode);
        }

        CORELIB_REFL_DECL_FIELD(m_negative);
        InputModifierNegate_sp m_negative{};

        CORELIB_REFL_DECL_FIELD(m_sizzleMode);
        InputModifierSwizzle_sp m_sizzleMode{};

    };
    CORELIB_DECL_SHORTSPTR(InputActionBinding);

    class InputActionMouseBinding : public InputActionBinding
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputActionMouseBinding, InputActionBinding);
    public:

    };
    CORELIB_DECL_SHORTSPTR(InputActionMouseBinding);

    class InputActionKeyboardBinding : public InputActionBinding
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputActionKeyboardBinding, InputActionBinding);
    public:
        string ToString() const override
        {
            return "Keyboard KeyCode: " + to_string(m_code);
        }

        InputActionValue GetValue();

        CORELIB_REFL_DECL_FIELD(m_code);
        KeyCode m_code = KeyCode::A;
    };
    CORELIB_DECL_SHORTSPTR(InputActionKeyboardBinding);

    class InputActionMap : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputActionMap, AssetObject);
        CORELIB_CLASS_ATTR(new CreateAssetAttribute);
    public:

        struct ActionBindingPair
        {
            InputValueType m_valueType{};
            array_list<SPtr<InputActionBinding>> m_bindings;
        };

        void OnConstruct() override;

        auto& GetActions() { return m_actions; }
        auto& GetActionNames() { return m_actionNames; }

        size_t GetActionCount() const { return m_actionNames->size(); }

        string GetActionNameAt(int index) const { return m_actionNames->at(index); }
        void SetActionNameAt(int index, string_view name) { m_actionNames->at(index) = name;}

        void SetActionAt(int index, const SPtr<ActionBindingPair>& action) { m_actions.at(index) = action; }
        SPtr<ActionBindingPair> GetActionAt(int index) const { return m_actions.at(index); }


        int NewAction(string_view name);
        void RemoveAction(string_view name);
        void RemoveActionAt(int index);

        bool IsValidIndex(int index) const;

    protected:
        array_list<SPtr<ActionBindingPair>> m_actions;
        List_sp<string> m_actionNames;
    };
    DECL_PTR(InputActionMap);

} // namespace pulsar
