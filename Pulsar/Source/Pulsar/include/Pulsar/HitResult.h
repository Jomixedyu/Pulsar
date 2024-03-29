#pragma once
#include "Components/Component.h"
#include "ObjectBase.h"

namespace pulsar
{
    struct Ray
    {
        Vector3f Origin;
        Vector3f Direction;
    };

    struct HitResult
    {
        Vector3f Start{};
        Vector3f End{};
        Vector3f Normal{};
        // position in world space
        Vector3f Position{};
        float    Distance{};
        int      TriFaceIndex{};
        ObjectPtr<Component> HitComponent;
        ObjectPtr<Node>      HitNode;
    };

}