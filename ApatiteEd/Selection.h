#pragma once
#include "Assembly.h"

namespace apatiteed
{
    class SelectionSet
    {
    public:
        ObjectBase_sp GetSelected();
        array_list<ObjectBase_sp> GetSelection();
        void Select(ObjectBase_rsp obj);
        void UnSelect(ObjectBase_rsp obj);
    protected:
        array_list<wptr<ObjectBase>> selection;
    };

}