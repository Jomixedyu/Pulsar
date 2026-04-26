#pragma once
#include "BPNode.h"
#include "PreDefine.h"
#include "BPConnect.h"

namespace bp
{

    //DCG

    class BPGraph
    {
    public:

        array_list<BPConnect> Connections;
        array_list<std::shared_ptr<BPNode>> Nodes;
    };
}