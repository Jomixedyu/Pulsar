#pragma once

#include <gfx/GFXDescriptorSet.h>
#include <gfx/GFXHandle.h>
#include <Pulsar/Assets/Material.h>
#include <Pulsar/Rendering/ShaderInstance.h>
#include <Pulsar/Rendering/ShaderPropertySheet.h>

#include <map>
#include <memory>
#include <string>

namespace pulsar
{
    // Per-pass binding key
    struct PassKey;

    // GPU-side counterpart of MaterialPassBinding, owned by RenderProxyMaterial
    struct RenderProxyMaterialPassBinding
    {
        std::shared_ptr<ShaderInstance> m_instance;

        gfx::GFXDescriptorSet_sp             m_descriptorSet;
        gfx::DescriptorSetLayoutHandle       m_descriptorSetLayout;
        gfx::GFXBuffer_sp                    m_materialConstantBuffer;
        bool                                 m_gpuResourcesInitialized = false;
        std::weak_ptr<ShaderProgramResource> m_builtWithProgram;

        std::shared_ptr<ShaderProgramResource> GetCurrentProgram() const
        {
            return m_instance ? m_instance->GetCurrentProgram() : nullptr;
        }
    };

    class RenderProxyMaterial
    {
    public:
        RenderProxyMaterial() = default;
        explicit RenderProxyMaterial(RCPtr<Material> source);
        explicit RenderProxyMaterial(Material* source);

        void SetSourceMaterial(RCPtr<Material> source);
        void SetSourceMaterial(Material* source);
        Material* GetSourceMaterial() const;

        // Called from RenderThread when the source material changes or proxy is created
        void InitRHI();
        void ReleaseRHI();

        // Renderer-triggered: called once per frame before drawing with this (pass, interface).
        // Returns nullptr if the shader for this binding is not yet ready.
        const RenderProxyMaterialPassBinding* PrepareForRendering(
            const std::string& passName,
            const std::string& interface_);

        // Called when source material parameters change
        void SubmitParameters(bool force = false);

    private:
        void ClearPassBindings();
        void EnsureGPUResources(RenderProxyMaterialPassBinding& binding, const ShaderPropertyLayout& layout);
        const RenderProxyMaterialPassBinding& GetPassBinding(
            const std::string& passName,
            const std::string& interface_);

    private:
        Material* m_sourceMaterial = nullptr;
        std::map<PassKey, RenderProxyMaterialPassBinding> m_passBindings;
        bool m_isDirtyParameter = false;
    };

    CORELIB_DECL_SHORTSPTR(RenderProxyMaterial);

} // namespace pulsar
