#pragma once
#include <Pulsar/ObjectBase.h>

namespace pulsar
{
    class MenuItemAttribute : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::MenuItemAttribute, Attribute);
    public:
        MenuItemAttribute(const char* path) : Path(path)
        {
        }

        const char* Path;
    };
    CORELIB_DECL_SHORTSPTR(MenuItemAttribute);
}