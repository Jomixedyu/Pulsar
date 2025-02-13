#pragma once
#include <Pulsar/AssetObject.h>

namespace pulsar
{
    struct CurveKey
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

        array_list<CurveKey> Keys;
    };

    class Curve : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Curve, AssetObject);
    public:

    };
}