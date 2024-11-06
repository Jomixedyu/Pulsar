#pragma once
#include "InputValueType.h"
#include "Pulsar/Assets/ScriptableAsset.h"

namespace pulsar
{
    class InputAction : public ScriptableAsset
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::InputAction, ScriptableAsset);
        CORELIB_CLASS_ATTR(new CreateAssetAttribute)
    public:
        InputAction()
        {
        }

        CORELIB_REFL_DECL_FIELD(m_consumeInput);
        bool m_consumeInput = true;

        CORELIB_REFL_DECL_FIELD(m_valueType);
        InputValueType m_valueType{};

    };
    DECL_PTR(InputAction);

} // namespace pulsar
