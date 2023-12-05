#include <gfx/GFXApplication.h>


gfx::GFXDescriptorSetLayout_sp gfx::GFXApplication::CreateDescriptorSetLayout(std::initializer_list<GFXDescriptorSetLayoutInfo> layouts)
{
    return CreateDescriptorSetLayout(layouts.begin(), layouts.size());
}