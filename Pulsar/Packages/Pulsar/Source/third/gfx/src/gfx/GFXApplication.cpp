#include <gfx/GFXApplication.h>

gfx::GFXApplication::~GFXApplication() = default;

gfx::GFXDescriptorSetLayout_sp gfx::GFXApplication::GetOrCreateDescriptorSetLayout(std::initializer_list<GFXDescriptorLayoutDesc> layouts)
{
    return GetOrCreateDescriptorSetLayout(layouts.begin(), layouts.size());
}