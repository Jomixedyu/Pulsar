#pragma once
#include <gfx/GFXResource.h>

#include <string>
#include <string_view>
#include <unordered_map>

namespace pulsar
{
    // 渲染线程的「已解析 bindable 表」:name -> live GPU 资源(GFXBuffer / GFXTexture2DView)。
    // 由各 set 的拥有者填充(set0 material / set1 perPass / set2 perDraw),
    // 交给 DescriptorSetAssembler 按反射 layout 装配 descriptor set。
    class RenderResourceRegistry
    {
    public:
        void Set(std::string_view name, gfx::GFXResource* res)
        {
            m_map[std::string(name)] = res;
        }

        gfx::GFXResource* Find(const std::string& name) const
        {
            auto it = m_map.find(name);
            return it != m_map.end() ? it->second : nullptr;
        }

        void Clear() { m_map.clear(); }

    private:
        std::unordered_map<std::string, gfx::GFXResource*> m_map;
    };
}
