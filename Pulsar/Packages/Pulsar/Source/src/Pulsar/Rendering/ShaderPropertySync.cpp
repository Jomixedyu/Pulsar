#include "ShaderPropertySync.h"
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

        return data;
    }

    void ShaderPropertySync::UploadPerMaterialCBuffer(
        const ShaderPropertyRenderData& data,
        const ShaderLayout& layout,
        gfx::GFXBuffer* cbuffer)
    {
        assert(Application::GetRenderThread()->IsRenderThread() && "UploadPerMaterialCBuffer must run on the render thread");

        if (!cbuffer)
            return;

        const ShaderPropertySetLayout* set0 = layout.FindSet(0);
        if (!set0)
            return;

        // 找到 set0 的材质 cbuffer（名为 PerMaterial 的 binding）
        const DescriptorBinding* matCbuffer = set0->FindBinding(kPerMaterialCBufferName);
        if (!matCbuffer)
            return;

        // 常量：按成员 offset 打包成字节缓冲，经 ring staging 队列上传到 device-local cbuffer
        // （帧头批量 transfer，无 per-update submit/wait）。
        std::vector<uint8_t> buffer(matCbuffer->m_size, 0);
        for (const auto& entry : matCbuffer->m_members)
        {
            auto it = data.Constants.find(entry.m_name);
            if (it != data.Constants.end() && it->second.Type == entry.m_type)
                WritePropertyToBuffer(buffer.data(), entry, it->second);
            // 没有值：保持 zero-initialized
        }
        Application::GetGfxApp()->RequestBufferUpload(cbuffer, buffer.data(), matCbuffer->m_size);
    }

    std::vector<gfx::GFXTexture2DView_sp> ShaderPropertySync::BuildSet0Registry(
        const ShaderPropertyRenderData& data,
        const ShaderPropertySetLayout& set0,
        gfx::GFXBuffer* cbuffer,
        RenderResourceRegistry& reg)
    {
        assert(Application::GetRenderThread()->IsRenderThread() && "BuildSet0Registry must run on the render thread");

        std::vector<gfx::GFXTexture2DView_sp> keepAlive;

        // set0 的材质 cbuffer binding（名为 PerMaterial）：绑定材质共享的那一份 cbuffer
        const DescriptorBinding* matCbuffer = set0.FindBinding(kPerMaterialCBufferName);
        if (cbuffer && matCbuffer)
            reg.Set(matCbuffer->m_name, cbuffer);

        auto* resMgr = Application::GetGfxApp()->GetResourceManager();
        for (const auto& b : set0.m_bindings)
        {
            if (b.IsBuffer())
                continue;

            // 缺失的纹理项：不写入 registry，交给 assembler 回落到 gfx 内建兜底
            auto it = data.Textures.find(b.m_name);
            if (it == data.Textures.end() || !it->second.IsValid())
                continue;

            if (auto* gfxTex = resMgr->GetTexture(it->second))
            {
                if (auto view = gfxTex->Get2DView(0))
                {
                    reg.Set(b.m_name, view.get());
                    keepAlive.push_back(std::move(view)); // registry 存裸指针，须保活到使用完毕
                }
            }
        }

        return keepAlive;
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
