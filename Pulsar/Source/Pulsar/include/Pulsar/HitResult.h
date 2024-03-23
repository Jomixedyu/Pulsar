#pragma once
#include "Components/Component.h"
#include "ObjectBase.h"

namespace pulsar
{
    struct HitResult
    {
        Vector3f Start;
        Vector3f End;
        Vector3f Normal;
        // position in world space
        Vector3f Position;
        float    Distance;
        int      TriFaceIndex;
        ObjectPtr<Component> HitComponent;
        ObjectPtr<Node>      HitNode;
    };

}