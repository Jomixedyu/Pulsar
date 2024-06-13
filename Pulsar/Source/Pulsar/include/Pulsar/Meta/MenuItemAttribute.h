#pragma once
#include <Pulsar/ObjectBase.h>

namespace pulsar
{
    class MenuItemAttribute : public Attribute
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::MenuItemAttribute, Attribute);
    public:
        explicit MenuItemAttribute(const char* path) : Path(path)
        {
        }

        const char* Path;
    };
    CORELIB_DECL_SHORTSPTR(MenuItemAttribute);
}