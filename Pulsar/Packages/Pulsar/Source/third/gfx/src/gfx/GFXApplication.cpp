#include <gfx/GFXApplication.h>

gfx::GFXApplication::~GFXApplication() = default;

gfx::GFXDescriptorSetLayout_sp gfx::GFXApplication::CreateDescriptorSetLayout(std::initializer_list<GFXDescriptorLayoutDesc> layouts)
{
    return CreateDescriptorSetLayout(layouts.begin(), layouts.size());
}