#pragma once
#include "Components/Component.h"
#include "ObjectBase.h"
#include "Rendering/Types.h"

namespace pulsar
{
    struct Ray
    {
        Vector3f Origin;
        Vector3f Direction;

        string to_string() const noexcept
        {
            return std::format("origin: {}, dir: {}", jmath::to_string(Origin), jmath::to_string(Direction));
        }
    };

    struct RaycastFilter
    {
        CullMode TriCullMode = CullMode::Front;
        array_list<ObjectPtr<Component>> IgnoreComponent;
        array_list<ObjectPtr<Node>>      IgnoreNode;
    };

    struct HitResult
    {
        Vector3f Start{};
        Vector3f End{};
        Vector3f Normal{};
        // position in world space
        Vector3f Position{};
        Vector2f Coordinate{};
        float    Distance{};
        int      TriFaceIndex{};
        ObjectPtr<Component> HitComponent;
        ObjectPtr<Node>      HitNode;
    };

}