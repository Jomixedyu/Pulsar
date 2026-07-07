#pragma once
#include <gfx/GFXResource.h>

#include <string>
#include <string_view>
#include <unordered_map>

namespace pulsar
{
    struct DescriptorBinding;

    // 渲染线程的「已解析 bindable 表」:name -> live GPU 资源(GFXBuffer / GFXTexture2DView)。
    // 由各 set 的拥有者填充(set0 material / set1 perPass / set2 perDraw),
    // 交给 DescriptorSetAssembler 按反射 layout 装配 descriptor set。
    //
    // registry 可挂一个 parent 形成 override 链:本层没有的 name 向 parent 逐级查找。
    // 链尾仍找不到时,Resolve 按 binding 类型/维度返回 gfx 引擎级内建兜底资源,
    // 保证每个反射到的 binding 永远有合法资源可绑。
    class RenderResourceRegistry
    {
    public:
        void Set(std::string_view name, gfx::GFXResource* res)
        {
            m_map[std::string(name)] = res;
        }

        // 设置 override 链的上层 registry;本层查不到的 name 会转交 parent。
        void SetParent(const RenderResourceRegistry* parent) { m_parent = parent; }

        // 仅按名查找,沿 parent 链上溯。全链未命中返回 nullptr。
        gfx::GFXResource* Find(const std::string& name) const
        {
            auto it = m_map.find(name);
            if (it != m_map.end())
                return it->second;
            return m_parent ? m_parent->Find(name) : nullptr;
        }

        // 完整解析一个反射 binding:先按名沿链查找,未命中则回落到与 binding
        // 类型/维度匹配的 gfx 内建兜底资源。对受支持的 binding 类型不会返回 nullptr。
        gfx::GFXResource* Resolve(const DescriptorBinding& binding) const;

        void Clear() { m_map.clear(); }

    private:
        std::unordered_map<std::string, gfx::GFXResource*> m_map;
        const RenderResourceRegistry* m_parent = nullptr;
    };
}
