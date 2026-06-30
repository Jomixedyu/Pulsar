#pragma once
#include <Pulsar/Rendering/RenderResourceRegistry.h>
#include <Pulsar/Rendering/ShaderPropertyLayout.h>

namespace gfx
{
    class GFXDescriptorSet;
}

namespace pulsar::DescriptorSetAssembler
{
    // 按反射 layout 把 registry 里的资源写入一个已存在的 descriptor set。
    // 逐 binding 按名取资源、按反射 type 绑定,最后 Submit。
    // set 的 layout 创建与 AllocateSet 由调用方负责。
    void Write(gfx::GFXDescriptorSet* set,
               const ShaderPropertySetLayout& setLayout,
               const RenderResourceRegistry& reg);
}
