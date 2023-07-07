#pragma once
#include <Pulsar/Assets/Material.h>

namespace pulsar::rendering
{
    class Pipeline
    {
    public:
        virtual Material_sp GetDefaultMaterial() = 0;

        
    };
}