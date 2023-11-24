#pragma once
#include "Curve.h"

namespace pulsar
{
    class CurveFloat : public Curve
    {

        float GetValue(float X) const
        {
            return 0;
            
        }

        CurveData m_curveData;
    };
}