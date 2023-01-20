#include "PropertyControl.h"

namespace apatiteed
{


    static inline auto manager()
    {
        static auto inst = new map<Type*, PropertyControl*>;
        return inst;
    }
    void PropertyControlManager::RegisterControl(Type* type, PropertyControl* control)
    {
        manager()->emplace(type, control);
    }
    PropertyControl* PropertyControlManager::FindControl(Type* type)
    {
        auto it = manager()->find(type);
        if (it == manager()->end())
        {
            return nullptr;
        }
        return it->second;
    }
}