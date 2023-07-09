#pragma once
#include "Assembly.h"
#include <CoreLib/Events.hpp>

namespace pulsared
{
    class SelectionSet
    {
    public:
        ObjectBase_sp GetSelected();
        array_list<ObjectBase_sp> GetSelection();
        void Select(ObjectBase_rsp obj);
        void UnSelect(ObjectBase_rsp obj);
        bool IsEmpty();
        void Clear();
        bool IsType(Type* type);
    protected:
        array_list<wptr<ObjectBase>> selection;
    };

}