#pragma once
#include <Pulsar/Rendering/ShaderPropertyValue.h>
#include <gfx/GFXHandle.h>

#include <map>
#include <string>

namespace pulsar
{
    // 渲染线程专用的着色器参数快照。
    //
    // ShaderPropertySheet 持有 RCPtr<Texture> 等游戏线程拥有的资产引用，渲染线程
    // 不应触碰。游戏线程负责把 sheet 解析成此结构：纹理解析成 GPU 句柄
    // （并确保 CreateGPUResource 在游戏线程触发），常量拷贝成纯值（按名字）。
    // 渲染线程消费此快照、按 layout 组装写入 GPU，不访问任何资产。
    //
    // 本结构与具体 layout 无关：常量与纹理都按属性名索引，渲染线程在 ApplyRenderData
    // 时再按 layout 的 offset / bindingPoint 组装。这样即便 layout 在 shader 编译完成
    // 后才确定，渲染线程也能用缓存快照完成首帧同步。
    struct ShaderPropertyRenderData
    {
        // 仅纯值（Int/Float/Float4），不含任何 RCPtr
        std::map<std::string, ShaderPropertyValue> Constants;
        // 已解析的纹理 GPU 句柄（值类型，跨线程安全）
        std::map<std::string, gfx::TextureHandle> Textures;
        // 游戏线程预解析的 fallback 纹理句柄（黑色），用于 layout 中存在但 sheet 未提供的纹理项
        gfx::TextureHandle FallbackTexture;
    };
}
