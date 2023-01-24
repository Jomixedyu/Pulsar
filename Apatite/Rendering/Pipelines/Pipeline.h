#pragma once
#include <Apatite/Assets/Material.h>

namespace apatite::rendering
{
    class Pipeline
    {
    public:
        virtual Material_sp GetDefaultMaterial() = 0;

        
    };
}