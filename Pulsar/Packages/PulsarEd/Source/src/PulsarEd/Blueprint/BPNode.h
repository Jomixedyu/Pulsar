#pragma once
#include "PreDefine.h"

namespace bp
{
    class BPGraph;

    class BPNode
    {
    public:
        
    public:
        array_list<string> InputPinsName;
        array_list<string> OutputPinsName;

        guid_t Guid;

        std::weak_ptr<BPGraph> Graph;
    };
}