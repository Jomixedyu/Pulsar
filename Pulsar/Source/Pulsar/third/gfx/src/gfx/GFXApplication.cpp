#include <gfx/GFXApplication.h>

gfx::GFXApplication::~GFXApplication() = default;

gfx::GFXRefCountPtr<gfx::GFXDescriptorSetLayout> gfx::GFXApplication::CreateDescriptorSetLayout(std::initializer_list<GFXDescriptorSetLayoutDesc> layouts)
{
    return CreateDescriptorSetLayout(layouts.begin(), layouts.size());
}