#pragma once
#include "Component.h"

namespace pulsar
{
    class ConstDataComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ConstDataComponent, Component);

    public:
        ConstDataComponent();

        CORELIB_REFL_DECL_FIELD(m_integer);
        int m_integer{};

        CORELIB_REFL_DECL_FIELD(m_string);
        SPtr<String> m_string;

        CORELIB_REFL_DECL_FIELD(m_float);
        float m_float{};
    };
}