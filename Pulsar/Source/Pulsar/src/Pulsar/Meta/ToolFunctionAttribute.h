#pragma once
#include <Pulsar/ObjectBase.h>

namespace pulsar
{
    class ToolFunctionAttribute : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ToolFunctionAttribute, Attribute);
    public:
        explicit ToolFunctionAttribute(const char* label = nullptr) : m_label(label)
        {
        }

        const char* GetLabel() const { return m_label ? m_label : ""; }

    private:
        const char* m_label = nullptr;
    };
    CORELIB_DECL_SHORTSPTR(ToolFunctionAttribute);
}
