#pragma once
#include "Assembly.h"
#include <CoreLib/Events.hpp>

namespace pulsared
{
    class SelectionSet
    {
    public:
        ObjectBase_ref GetSelected();
        array_list<ObjectBase_ref> GetSelection();
        void Select(ObjectBase_ref obj);
        void UnSelect(ObjectBase_ref obj);
        bool IsEmpty();
        void Clear();
        bool IsType(Type* type);
    protected:
        array_list<ObjectBase_ref> selection;
    };

}