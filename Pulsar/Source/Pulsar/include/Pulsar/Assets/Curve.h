#pragma once
#include <Pulsar/AssetObject.h>

namespace pulsar
{
    struct CurveKeyInfo
    {
        float X;
        float Y;
    };
    struct CurveData
    {
    public:
        float Sample(float X) const
        {
            return 0;
        }

        array_list<CurveKeyInfo> Keys;
    };
    class Curve
    {

    };
}