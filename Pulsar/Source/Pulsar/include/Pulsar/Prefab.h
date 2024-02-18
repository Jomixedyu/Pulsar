#pragma once
#include "Assets/NodeCollection.h"

namespace pulsar
{
    class Prefab : public NodeCollection
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Prefab, NodeCollection);

    public:

        static ObjectPtr<Prefab> StaticCreate(string_view name);
    };
    DECL_PTR(Prefab);

}