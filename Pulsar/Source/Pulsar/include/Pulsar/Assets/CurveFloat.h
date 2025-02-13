#pragma once
#include "Curve.h"

namespace pulsar
{
    class CurveFloat : public Curve
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::CurveFloat, Curve);
    public:
        float GetValue(float X) const
        {
            return 0;
            
        }

        CurveData m_curveData;
    };
}