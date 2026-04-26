#pragma once
#include "EngineMath.h"
#include <CoreLib.Serialization/ObjectSerializer.h>
#include "Assembly.h"

namespace pulsar
{
    class AssetSerializerUtil
    {
    public:
        static ser::VarientRef NewObject(const ser::VarientRef& ctx, Vector3f vec);
        static ser::VarientRef NewObject(const ser::VarientRef& ctx, Vector4f vec);
        static ser::VarientRef NewObject(const ser::VarientRef& ctx, const BoxSphereBounds3f& vec);

        static Vector3f GetVector3Object(const ser::VarientRef& var);
        static Vector4f GetVector4Object(const ser::VarientRef& var);
        static BoxSphereBounds3f GetBounds3Object(const ser::VarientRef& var);
    };
}