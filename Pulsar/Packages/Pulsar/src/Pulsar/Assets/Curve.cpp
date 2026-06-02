#include "Pulsar/Assets/Curve.h"

#include "CoreLib.Serialization/JsonSerializer.h"

namespace pulsar
{
    template<typename T>
    static T Lerp(T a, T b, float t)
    {
        return a + t * (b - a);
    }

    float CurveKeyCollection::Sample(float InTime, float InDefaultValue) const
    {
        if (Keys.empty())
        {
            return InDefaultValue;
        }
        if (Keys.size() == 1)
        {
            return Keys[0].Value;
        }

        if (InTime <= Keys.front().Time)
        {
            return Keys.front().Value;
        }
        if (InTime >= Keys.back().Time)
        {
            return Keys.back().Value;
        }

        for (size_t i = 0; i < Keys.size() - 1; ++i)
        {
            if (InTime >= Keys[i].Time && InTime <= Keys[i + 1].Time)
            {
                auto keyA = Keys[i];
                auto keyB = Keys[i + 1];
                auto dt = keyB.Time - keyA.Time;
                if (dt == 0)
                {
                    return keyA.Value;
                }
                auto t = (InTime - keyA.Time) / dt;
                return Lerp(keyA.Value, keyB.Value, t);
            }
        }

        return InDefaultValue;
    }

} // namespace pulsar