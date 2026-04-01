#include "ShaderPropertySync.h"
#include "BuiltinAsset.h"
#include "Assets/Texture2D.h"

#include "Application.h"
#include "AppInstance.h"
#include "AssetManager.h"
#include <Pulsar/Logger.h>

#include <gfx/GFXTexture.h>

#include <cstring>

namespace pulsar
{
    void ShaderPropertySync::SyncSheetToGpu(
        const ShaderPropertySheet& sheet,
        const ShaderPropertyLayout& layout,
        gfx::GFXBuffer* cbuffer,
        gfx::GFXDescriptorSet* descriptorSet)
    {
        SyncConstants(sheet, layout, cbuffer);
        SyncTextures(sheet, layout, descriptorSet);
    }

    void ShaderPropertySync::SyncConstants(
        const ShaderPropertySheet& sheet,
        const ShaderPropertyLayout& layout,
        gfx::GFXBuffer* cbuffer)
    {
        if (!cbuffer || layout.m_totalCBufferSize == 0)
            return;

        std::vector<uint8_t> buffer(layout.m_totalCBufferSize, 0);

        for (const auto& entry : layout.m_constantEntries)
        {
            const ShaderPropertyValue* prop = sheet.FindProperty(entry.m_name);

            auto isCompatible = [](ShaderPropertyType a, ShaderPropertyType b) {
                if (a == b) return true;
                // Float4 和 Color 内存布局相同，互相兼容
                if ((a == ShaderPropertyType::Float4 || a == ShaderPropertyType::Color) &&
                    (b == ShaderPropertyType::Float4 || b == ShaderPropertyType::Color))
                    return true;
                return false;
            };

            if (prop && isCompatible(entry.m_type, prop->Type))
            {
                WritePropertyToBuffer(buffer.data(), entry, *prop);
            }
            // 没有值：保持 zero-initialized（DefaultValue 应已由 ApplyShaderDefaults 写入 sheet）
        }

        cbuffer->Fill(buffer.data());
    }

    void ShaderPropertySync::SyncTextures(
        const ShaderPropertySheet& sheet,
        const ShaderPropertyLayout& layout,
        gfx::GFXDescriptorSet* descriptorSet)
    {
        if (!descriptorSet)
            return;

        auto assetMgr = AssetManager::Get();

        for (const auto& entry : layout.m_textureEntries)
        {
            RCPtr<Texture> tex;
            sheet.GetTexture(entry.m_name, tex);

            // 没找到时 fallback 白色纹理，保证 descriptor 永远被 update
            if (!tex)
            {
                RCPtr<Texture2D> defaultTex = BuiltinAsset::GetTextureBlack();
                if (defaultTex) tex = defaultTex;
            }

            if (tex && !tex->IsCreatedGPUResource())
                tex->CreateGPUResource();

            if (tex && tex->GetGFXTexture())
            {
                auto* descriptor = descriptorSet->FindByBinding(entry.m_bindingPoint);
                if (descriptor)
                {
                    auto view = tex->GetGFXTexture()->Get2DView(0);
                    if (view)
                        descriptor->SetTextureSampler2D(view.get());
                    else
                        Logger::Log("SyncTextures: view is null, binding=" + std::to_string(entry.m_bindingPoint), LogLevel::Warning);
                }
                else
                    Logger::Log("SyncTextures: FindByBinding(" + std::to_string(entry.m_bindingPoint) + ") returned null", LogLevel::Warning);
            }
            else
                Logger::Log("SyncTextures: no valid texture for entry=" + entry.m_name, LogLevel::Warning);
        }
    }

    void ShaderPropertySync::WritePropertyToBuffer(uint8_t* buffer, const CBufferEntry& entry, const ShaderPropertyValue& prop)
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
        case ShaderPropertyType::Float4:
        case ShaderPropertyType::Color: {
            float vals[4] = {};
            if (prop.Type == ShaderPropertyType::Color) {
                Color4f val = prop.AsColor();
                vals[0] = val.r; vals[1] = val.g; vals[2] = val.b; vals[3] = val.a;
            } else {
                Vector4f val = prop.AsFloat4();
                vals[0] = val.x; vals[1] = val.y; vals[2] = val.z; vals[3] = val.w;
            }
            std::memcpy(buffer + entry.m_offset, vals, sizeof(vals));
            break;
        }
        default:
            break;
        }
    }

}