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

    // Per-pass binding: holds a ShaderInstance for a specific (pass, interface) combination
    struct MaterialPassBinding
    {
        std::shared_ptr<ShaderInstance> m_instance;

        std::shared_ptr<ShaderProgramResource> GetCurrentProgram() const
        {
            return m_instance ? m_instance->GetCurrentProgram() : nullptr;
        }
    };

    class Material final : public AssetObject, public IGPUResource
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Material, AssetObject);
        CORELIB_CLASS_ATTR(new CreateAssetAttribute("Engine/Materials/Error"));

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

        void SubmitParameters(bool force = false);

        // Per-pass binding: lazily creates ShaderInstance for (pass, interface)
        const MaterialPassBinding& GetPassBinding(
            const std::string& passName,
            const std::string& interface_);

        // Per-material descriptor set (set 0)
        gfx::GFXDescriptorSet_sp GetDescriptorSet() const { return m_descriptorSet; }
        gfx::GFXDescriptorSetLayout_sp GetDescriptorSetLayout() const { return m_descriptorSetLayout; }

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
        void EnsureGPUResources(const ShaderPropertyLayout& layout);

    private:
        CORELIB_REFL_DECL_FIELD(m_shader);
        RCPtr<Shader> m_shader;

        ShaderPropertySheet m_sheet;
        std::vector<std::string> m_activeFeatures;

        // Per-pass ShaderInstance cache (lazy-created)
        std::map<PassKey, MaterialPassBinding> m_passBindings;

        // Per-material GPU resources (set 0, shared by all passes)
        gfx::GFXDescriptorSet_sp             m_descriptorSet;
        gfx::GFXDescriptorSetLayout_sp       m_descriptorSetLayout;
        gfx::GFXBuffer_sp                    m_materialConstantBuffer;

        bool m_createdGpuResource = false;
        bool m_gpuResourcesInitialized = false;
        std::weak_ptr<ShaderProgramResource> m_builtWithProgram; // tracks which program GPU resources were built from
        bool m_isDirtyParameter{};
    };

} // namespace pulsar
