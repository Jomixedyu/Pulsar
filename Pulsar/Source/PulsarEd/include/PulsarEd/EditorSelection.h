#pragma once
#include "SelectionSet.h"
#include <Pulsar/Node.h>

namespace pulsared
{

    class EditorSelection
    {
    public:
        static inline SelectionSet<Node> Selection;
    };
}