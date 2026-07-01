#include "ShaderPropertySync.h"
#include "BuiltinAsset.h"
#include "Assets/Texture2D.h"
#include "Rendering/DescriptorSetAssembler.h"
#include "Rendering/RenderResourceRegistry.h"

#include "Application.h"
#include "AppInstance.h"
#include "AssetManager.h"
#include <Pulsar/Logger.h>
#include <Pulsar/Rendering/RenderThread.h>

#include <gfx/GFXTexture.h>
#include <gfx/GFXResourceManager.h>

#include <cassert>
#include <cstring>

namespace pulsar
{
    ShaderPropertyRenderData ShaderPropertySync::BuildRenderData(const ShaderPropertySheet& sheet)
    {
        // 在 sheet 拥有者线程调用：解析纹理资产为 GPU 句柄、拷贝常量纯值。
        // 场景 material 由游戏线程驱动；post-process material 由渲染线程在构建
        // RenderGraph 时驱动。lockstep 下两线程互不重叠，均可安全访问资产。
        ShaderPropertyRenderData data;

        for (const auto& [name, prop] : sheet.GetAllProperties())
        {
            switch (prop.Type)
            {
            case ShaderPropertyType::Int:
            case ShaderPropertyType::Float:
            case ShaderPropertyType::Float4:
                data.Constants.emplace(name, prop);
                break;
            case ShaderPropertyType::Texture2D:
            {
                RCPtr<Texture> tex = prop.AsTexture2D();
                if (tex && !tex->IsCreatedGPUResource())
                    tex->CreateGPUResource();
                data.Textures.emplace(name, tex ? tex->GetTextureHandle() : gfx::TextureHandle{});
                break;
            }
            default:
                break;
            }
        }

        // 预解析 fallback：layout 中存在但 sheet 未提供的纹理项，渲染线程用此句柄兜底
        RCPtr<Texture2D> fallback = BuiltinAsset::GetTextureBlack();
        if (fallback)
        {
            if (!fallback->IsCreatedGPUResource())
                fallback->CreateGPUResource();
            data.FallbackTexture = fallback->GetTextureHandle();
        }

        return data;
    }

    void ShaderPropertySync::ApplyRenderData(
        const ShaderPropertyRenderData& data,
        const ShaderLayout& layout,
        gfx::GFXBuffer* cbuffer,
        gfx::GFXDescriptorSet* descriptorSet)
    {
        assert(Application::GetRenderThread()->IsRenderThread() && "ApplyRenderData must run on the render thread");

        const ShaderPropertySetLayout* set0 = layout.FindSet(0);
        if (!set0)
            return;

        // 找到 set0 的材质 cbuffer（首个有尺寸的 buffer binding）
        const DescriptorBinding* matCbuffer = nullptr;
        for (const auto& b : set0->m_bindings)
        {
            if (b.IsBuffer() && b.m_size > 0)
            {
                matCbuffer = &b;
                break;
            }
        }

        // 常量：按成员 offset 打包成字节缓冲并上传
        if (cbuffer && matCbuffer)
        {
            std::vector<uint8_t> buffer(matCbuffer->m_size, 0);
            for (const auto& entry : matCbuffer->m_members)
            {
                auto it = data.Constants.find(entry.m_name);
                if (it != data.Constants.end() && it->second.Type == entry.m_type)
                    WritePropertyToBuffer(buffer.data(), entry, it->second);
                // 没有值：保持 zero-initialized
            }
            cbuffer->Fill(buffer.data());
        }

        if (!descriptorSet)
            return;

        // 填入渲染资源注册表：cbuffer + 已解析纹理 view（含 fallback），交给 assembler 按反射装配
        RenderResourceRegistry reg;
        if (cbuffer && matCbuffer)
            reg.Set(matCbuffer->m_name, cbuffer);

        auto* resMgr = Application::GetGfxApp()->GetResourceManager();
        for (const auto& b : set0->m_bindings)
        {
            if (b.IsBuffer())
                continue;

            gfx::TextureHandle handle{};
            auto it = data.Textures.find(b.m_name);
            if (it != data.Textures.end() && it->second.IsValid())
                handle = it->second;
            else
                handle = data.FallbackTexture;

            if (!handle.IsValid())
                continue;

            if (auto* gfxTex = resMgr->GetTexture(handle))
            {
                if (auto view = gfxTex->Get2DView(0))
                    reg.Set(b.m_name, view.get());
            }
        }

        DescriptorSetAssembler::Write(descriptorSet, *set0, reg);
    }

    void ShaderPropertySync::WritePropertyToBuffer(uint8_t* buffer, const BufferMember& entry, const ShaderPropertyValue& prop)
    {
        switch (entry.m_type)
        {
        case ShaderPropertyType::Int: {
            int val = prop.AsInt();
            std::memcpy(buffer + entry.m_offset, &val, sizeof(int));
            break;
        }
        case ShaderPropertyType::Float: {
            float val = prop.AsFloat();
            std::memcpy(buffer + entry.m_offset, &val, sizeof(float));
            break;
        }
        case ShaderPropertyType::Float4: {
            Vector4f val = prop.AsFloat4();
            std::memcpy(buffer + entry.m_offset, &val, sizeof(Vector4f));
            break;
        }
        default:
            break;
        }
    }

}
