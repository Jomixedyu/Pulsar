#include "RenderProxyMaterial.h"

#include <Pulsar/Application.h>
#include <Pulsar/AssetManager.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Assets/Texture2D.h>
#include <Pulsar/Rendering/ShaderInstanceCache.h>
#include <Pulsar/Rendering/ShaderPropertySync.h>
#include <Pulsar/Rendering/RenderThread.h>
#include <gfx/GFXResourceManager.h>

namespace pulsar
{

    RenderProxyMaterial::RenderProxyMaterial(RCPtr<Material> source)
        : m_sourceMaterial(source.GetPtr())
    {
    }

    RenderProxyMaterial::RenderProxyMaterial(Material* source)
        : m_sourceMaterial(source)
    {
    }

    void RenderProxyMaterial::SetSourceMaterial(RCPtr<Material> source)
    {
        m_sourceMaterial = source.GetPtr();
        ClearPassBindings();
    }

    void RenderProxyMaterial::SetSourceMaterial(Material* source)
    {
        m_sourceMaterial = source;
        ClearPassBindings();
    }

    Material* RenderProxyMaterial::GetSourceMaterial() const
    {
        return m_sourceMaterial;
    }

    void RenderProxyMaterial::InitRHI()
    {
        // GPU resources are lazily created per-pass-binding in PrepareForRendering
    }

    void RenderProxyMaterial::ReleaseRHI()
    {
        ClearPassBindings();
    }

    void RenderProxyMaterial::ClearPassBindings()
    {
        m_passBindings.clear();
    }

    const RenderProxyMaterialPassBinding* RenderProxyMaterial::PrepareForRendering(
        const std::string& passName,
        const std::string& interface_)
    {
        if (!m_sourceMaterial)
            return nullptr;

        auto& binding = const_cast<RenderProxyMaterialPassBinding&>(GetPassBinding(passName, interface_));

        auto program = binding.GetCurrentProgram();
        if (!program)
            return nullptr;

        if (program != binding.m_builtWithProgram.lock())
        {
            binding.m_descriptorSet.reset();
            binding.m_descriptorSetLayout.Reset();
            binding.m_materialConstantBuffer.reset();
            binding.m_gpuResourcesInitialized = false;

            RenderThread::Get().EnqueueCommandSync([this, &binding, &program]() {
                this->EnsureGPUResources(binding, program->m_layout);
            });
            binding.m_builtWithProgram = program;

            // Initial parameter sync
            if (m_sourceMaterial)
            {
                m_sourceMaterial->ApplyShaderDefaults();
                ShaderPropertySync::SyncSheetToGpu(
                    m_sourceMaterial->GetSheet(),
                    program->m_layout,
                    binding.m_materialConstantBuffer.get(),
                    binding.m_descriptorSet.get());
                if (binding.m_descriptorSet)
                    binding.m_descriptorSet->Submit();
            }
        }

        if (!binding.m_gpuResourcesInitialized)
            return nullptr;

        return &binding;
    }

    void RenderProxyMaterial::SubmitParameters(bool force)
    {
        if (!m_sourceMaterial)
            return;

        if (!m_isDirtyParameter && !force)
            return;

        m_sourceMaterial->ApplyShaderDefaults();

        bool anyUploaded = false;
        for (auto& [key, binding] : m_passBindings)
        {
            if (!binding.m_gpuResourcesInitialized) continue;

            auto program = binding.GetCurrentProgram();
            if (!program) continue;

            ShaderPropertySync::SyncSheetToGpu(
                m_sourceMaterial->GetSheet(),
                program->m_layout,
                binding.m_materialConstantBuffer.get(),
                binding.m_descriptorSet.get());

            if (binding.m_descriptorSet)
                binding.m_descriptorSet->Submit();

            anyUploaded = true;
        }

        if (anyUploaded)
            m_isDirtyParameter = false;
    }

    const RenderProxyMaterialPassBinding& RenderProxyMaterial::GetPassBinding(
        const std::string& passName,
        const std::string& interface_)
    {
        PassKey key{passName, interface_};
        auto it = m_passBindings.find(key);
        if (it != m_passBindings.end())
            return it->second;

        if (!m_sourceMaterial)
        {
            static RenderProxyMaterialPassBinding empty{};
            return empty;
        }

        auto shader = m_sourceMaterial->GetShader();
        if (!shader)
        {
            static RenderProxyMaterialPassBinding empty{};
            return empty;
        }

        auto config = shader->GetConfig();

        ShaderVariantKey variantKey;
        variantKey.m_shaderGuid = shader->GetAssetGuid();
        variantKey.m_passName = passName;
        variantKey.m_interface = interface_;
        variantKey.m_features = m_sourceMaterial->GetActiveFeatures();

        ShaderCompileTask task;
        task.m_variantKey = variantKey;

        if (config && config->Passes)
        {
            for (const auto& pass : *config->Passes)
            {
                if (pass->Name == passName && pass->Entry)
                {
                    task.m_entries.m_vertex = pass->Entry->Vertex;
                    task.m_entries.m_fragment = pass->Entry->Fragment;
                    task.m_entries.m_tessControl = pass->Entry->TessControl;
                    task.m_entries.m_tessEval = pass->Entry->TessEval;
                    break;
                }
            }

            if (task.m_entries.m_vertex.empty() && task.m_entries.m_fragment.empty())
            {
                for (const auto& pass : *config->Passes)
                {
                    if (pass->Entry)
                    {
                        task.m_entries.m_vertex = pass->Entry->Vertex;
                        task.m_entries.m_fragment = pass->Entry->Fragment;
                        task.m_entries.m_tessControl = pass->Entry->TessControl;
                        task.m_entries.m_tessEval = pass->Entry->TessEval;
                        break;
                    }
                }
            }
        }

        auto instance = ShaderInstanceCache::Instance().GetOrCreate(variantKey, task);
        RenderProxyMaterialPassBinding binding;
        binding.m_instance = instance;
        auto [insertIt, _] = m_passBindings.emplace(key, std::move(binding));

        return insertIt->second;
    }

    void RenderProxyMaterial::EnsureGPUResources(
        RenderProxyMaterialPassBinding& binding,
        const ShaderPropertyLayout& layout)
    {
        if (binding.m_gpuResourcesInitialized)
            return;

        auto gfxApp = Application::GetGfxApp();

        array_list<gfx::GFXDescriptorSetLayoutDesc> descLayoutInfos;

        if (layout.m_totalCBufferSize > 0)
        {
            gfx::GFXDescriptorSetLayoutDesc cbDesc{};
            cbDesc.Type = gfx::GFXDescriptorType::ConstantBuffer;
            cbDesc.Stage = (layout.m_cbufferStageFlags != gfx::GFXGpuProgramStageFlags::None)
                ? layout.m_cbufferStageFlags
                : gfx::GFXGpuProgramStageFlags::VertexFragment;
            cbDesc.BindingPoint = layout.m_cbufferBindingPoint;
            descLayoutInfos.push_back(cbDesc);

            gfx::GFXBufferDesc bufferDesc{};
            bufferDesc.Usage = gfx::GFXBufferUsage::ConstantBuffer;
            bufferDesc.StorageType = gfx::GFXBufferMemoryPosition::VisibleOnDevice;
            bufferDesc.BufferSize = layout.m_totalCBufferSize;
            binding.m_materialConstantBuffer = gfxApp->CreateBuffer(bufferDesc);
        }

        for (const auto& texEntry : layout.m_textureEntries)
        {
            gfx::GFXDescriptorSetLayoutDesc texDesc{};
            texDesc.Type = texEntry.m_isCombinedImageSampler
                ? gfx::GFXDescriptorType::CombinedImageSampler
                : gfx::GFXDescriptorType::Texture2D;
            texDesc.Stage = texEntry.m_stageFlags;
            texDesc.BindingPoint = texEntry.m_bindingPoint;
            descLayoutInfos.push_back(texDesc);
        }

        auto& cmdList = Application::GetGfxApp()->GetImmediateCommandList();

        binding.m_descriptorSetLayout = cmdList.CreateDescriptorSetLayout(descLayoutInfos);
        binding.m_descriptorSet = gfxApp->GetDescriptorManager()->GetDescriptorSet(binding.m_descriptorSetLayout.Lock());

        if (binding.m_materialConstantBuffer)
        {
            binding.m_descriptorSet->AddDescriptor("ConstantProperties", 0)
                ->SetConstantBuffer(binding.m_materialConstantBuffer.get());
        }

        for (const auto& texEntry : layout.m_textureEntries)
        {
            binding.m_descriptorSet->AddDescriptor(texEntry.m_name, texEntry.m_bindingPoint);
        }

        binding.m_gpuResourcesInitialized = true;
    }

} // namespace pulsar
