#pragma once
#include "PreDefine.h"

namespace bp
{

    struct BPPin
    {
        guid_t Guid;
        int Index;
    };

    struct BPConnect
    {
    public:
        BPPin A;
        BPPin B;
    };
}