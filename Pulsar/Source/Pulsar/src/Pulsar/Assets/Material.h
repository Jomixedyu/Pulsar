#pragma once

#include <gfx/GFXDescriptorSet.h>
#include <Pulsar/Rendering/ShaderPropertySheet.h>
#include <Pulsar/Rendering/ShaderInstance.h>
#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Assets/Texture.h>

#include <map>
#include <memory>
#include <string>
#include <vector>

namespace pulsar
{
    // Per-pass binding key
    struct PassKey
    {
        std::string m_passName;
        std::string m_interface;

        bool operator==(const PassKey& other) const = default;
        bool operator<(const PassKey& other) const
        {
            if (m_passName != other.m_passName) return m_passName < other.m_passName;
            return m_interface < other.m_interface;
        }
    };

    // Per-pass binding: holds a ShaderInstance AND its own GPU resources for a specific (pass, interface) combination
    struct MaterialPassBinding
    {
        std::shared_ptr<ShaderInstance> m_instance;

        // Per-binding GPU resources (lazily created when this interface's shader is ready)
        gfx::GFXDescriptorSet_sp             m_descriptorSet;
        gfx::GFXDescriptorSetLayout_sp       m_descriptorSetLayout;
        gfx::GFXBuffer_sp                    m_materialConstantBuffer;
        bool                                 m_gpuResourcesInitialized = false;
        std::weak_ptr<ShaderProgramResource> m_builtWithProgram;

        std::shared_ptr<ShaderProgramResource> GetCurrentProgram() const
        {
            return m_instance ? m_instance->GetCurrentProgram() : nullptr;
        }
    };

    class Material final : public AssetObject, public IGPUResource
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Material, AssetObject);
        CORELIB_CLASS_ATTR(
            new CreateAssetAttribute("Engine/Materials/Error"),
            new AssetIconAttribute("Editor/Icons/material.png")
            );

    public:
        static RCPtr<Material> StaticCreate(const RCPtr<Shader>& shader, string_view name = {});

        virtual void Serialize(AssetSerializer* s) override;
        void OnInstantiateAsset(AssetObject* obj) override;
    public:
        virtual bool CreateGPUResource() override;
        virtual void DestroyGPUResource() override;
        virtual bool IsCreatedGPUResource() const override;

        void OnCollectAssetDependencies(array_list<jxcorlib::guid_t> &deps) override;
        void GetSubscribeObserverHandles(array_list<ObjectHandle>& out) override;

    protected:
        void OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg) override;
    public:
        // Parameter accessors (operate on m_sheet)
        void SetIntScalar(const index_string& name, int value);
        void SetFloat(const index_string& name, float value);
        void SetTexture(const index_string& name, const RCPtr<Texture>& value);
        void SetVector4(const index_string& name, const Vector4f& value);
        int GetIntScalar(const index_string& name);
        float GetScalar(const index_string& name);
        Vector4f GetVector4(const index_string& name);
        RCPtr<Texture> GetTexture(const index_string& name);

        // User-triggered: upload dirty parameters to all ready bindings.
        // Call this after modifying material parameters (SetFloat / SetTexture / etc.).
        void SubmitParameters(bool force = false);

        // Renderer-triggered: called once per frame before drawing with this (pass, interface).
        // Detects async shader compilation completing, creates GPU resources, and does the
        // initial parameter sync into freshly created resources.
        // Returns nullptr if the shader for this binding is not yet ready.
        const MaterialPassBinding* PrepareForRendering(
            const std::string& passName,
            const std::string& interface_);

        // Per-pass binding: lazily creates ShaderInstance for (pass, interface)
        // The binding owns its own GPU resources (descriptor set, cbuffer), created when the shader is ready.
        const MaterialPassBinding& GetPassBinding(
            const std::string& passName,
            const std::string& interface_);

    public:
        RCPtr<Shader> GetShader() const;
                void SetShader(RCPtr<Shader> value);
                void ApplyShaderDefaults();

        const std::vector<std::string>& GetActiveFeatures() const { return m_activeFeatures; }
        void SetActiveFeatures(std::vector<std::string> features);

        ShaderPropertySheet& GetSheet() { return m_sheet; }
        const ShaderPropertySheet& GetSheet() const { return m_sheet; }

        Action<> OnShaderChanged;

    protected:
        void PostEditChange(FieldInfo* info) override;

    private:
        void ClearPassBindings();
        void EnsureGPUResources(MaterialPassBinding& binding, const ShaderPropertyLayout& layout);

    private:
        CORELIB_REFL_DECL_FIELD(m_shader);
        RCPtr<Shader> m_shader;

        ShaderPropertySheet m_sheet;
        std::vector<std::string> m_activeFeatures;

        // Per-pass ShaderInstance cache (lazy-created); each binding owns its own GPU resources
        std::map<PassKey, MaterialPassBinding> m_passBindings;

        bool m_createdGpuResource = false;
        bool m_isDirtyParameter{};
    };

} // namespace pulsar
