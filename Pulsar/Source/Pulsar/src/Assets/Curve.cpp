#include "Pulsar/Assets/Curve.h"

#include "CoreLib.Serialization/JsonSerializer.h"

namespace pulsar
{
    template<typename T>
    static T Lerp(T a, T b, float t)
    {
        return a + t * (b - a);
    }

    float CurveData::Sample(float InTime, float InDefaultValue) const
    {
        auto keyA = Keys->at(0);
        auto keyB = Keys->at(1);

        auto dt = keyB.Time - keyA.Time;
        auto t = (InTime - keyA.Time) / dt;
        return Lerp(keyA.Value, keyB.Value, t);
    }

} // namespace pulsar