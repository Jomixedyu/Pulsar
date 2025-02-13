#pragma once
#include "Curve.h"

namespace pulsar
{
    class CurveLinearColor : public Curve
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::CurveLinearColor, Curve);
    public:

        Color4f SampleColor(float t) const;

        CurveData m_curves[4];
    };
}