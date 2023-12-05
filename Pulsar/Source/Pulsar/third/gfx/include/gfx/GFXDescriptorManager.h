#pragma once
#include "GFXDescriptorSet.h"
#include <vector>
#include <cstdint>
#include <memory>

namespace gfx
{
    class GFXDescriptorManager
    {
    public:
        virtual std::shared_ptr<GFXDescriptorSet> GetDescriptorSet(GFXDescriptorSetLayout_sp layout) = 0;

        virtual ~GFXDescriptorManager() {}
    protected:
    };
}