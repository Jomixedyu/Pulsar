#pragma once
#include "BrushEdTool.h"
#include "Pulsar/HitResult.h"

namespace pulsared
{

    class MeshVertexBrushProperties
    {
    public:
        float Size;
        float Strength;
        float Falloff;
    };



    class MeshVertexBrush : public BrushEdTool
    {
        using base = BrushEdTool;
    public:
        virtual bool HitTest(const Ray& ray, HitResult& result);
    };

} // namespace pulsared
