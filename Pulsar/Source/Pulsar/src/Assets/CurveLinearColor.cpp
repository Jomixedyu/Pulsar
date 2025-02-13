#include "Pulsar/Assets/CurveLinearColor.h"

namespace pulsar
{

    Color4f CurveLinearColor::SampleColor(float t) const
    {
        Color4f color;
        color.r = m_curves[0].Sample(t);
        color.g = m_curves[1].Sample(t);
        color.b = m_curves[2].Sample(t);
        color.a = m_curves[3].Sample(t);

        return color;
    }
} // namespace pulsar