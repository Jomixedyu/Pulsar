#pragma once

#include <gfx/GFXDescriptorSet.h>

#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Assets/Texture.h>
#include <variant>

namespace pulsar
{
    template <typename T>
    struct NotNullOption
    {
        explicit NotNullOption(const T& value ,const T& defaultValue = {}) : m_value(value),  m_default(defaultValue)
        {
        }
        bool HasValue() const noexcept { return (bool)m_default; }

        T& Get() const noexcept { return HasValue() ? m_value : m_default; }

        T m_value;
        T m_default;
    };


    class Material final : public AssetObject, public IGPUResource
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Material, AssetObject);
        CORELIB_CLASS_ATTR(new CreateAssetAttribute(BuiltinAsset::Material_Missing));

    public:
        static RCPtr<Material> StaticCreate(string_view name);

        virtual void Serialize(AssetSerializer* s) override;
        void OnInstantiateAsset(AssetObject* obj) override;
    public:
        virtual bool CreateGPUResource() override;
        virtual void DestroyGPUResource() override;
        virtual bool IsCreatedGPUResource() const override;
        void GetSubscribeObserverHandles(array_list<ObjectHandle>& out) override;

    protected:
        void OnNotifyObserver(ObjectHandle inDependency, DependencyObjectState msg) override;
        void ActiveShader();
        void InactiveShader();
    public:
        void ClearUnusedParameterValue();
        void SetIntScalar(const index_string& name, int value);
        void SetFloat(const index_string& name, float value);
        void SetTexture(const index_string& name, const RCPtr<Texture>& value);
        void SetVector4(const index_string& name, const Vector4f& value);
        int GetIntScalar(const index_string& name);
        float GetScalar(const index_string& name);
        Vector4f GetVector4(const index_string& name);
        RCPtr<Texture> GetTexture(const index_string& name);

        void SubmitParameters(bool force = false);

        gfx::GFXDescriptorSet_sp GetGfxDescriptorSet() const
        {
            return m_descriptorSet;
        }
        gfx::GFXDescriptorSetLayout_sp GetGfxDescriptorSetLayout() const
        {
            return m_descriptorSetLayout;
        }
        const auto& GetGpuPrograms() const { return m_gpuPrograms; }
        const auto& GetPSOParams() const { return m_psoState; }
    public:
        RCPtr<Shader> GetShader() const;
        void SetShader(RCPtr<Shader> value);

        Action<> OnShaderChanged;


    protected:
        void PostEditChange(FieldInfo* info) override;

    private:
        CORELIB_REFL_DECL_FIELD(m_shader);
        RCPtr<Shader> m_shader;

        RCPtr<Shader> m_submitShader;

        array_list<gfx::GFXGpuProgram_sp>    m_gpuPrograms;
        gfx::GFXGraphicsPipelineStateParams  m_psoState;

        std::vector<uint8_t>                 m_bufferData;

        gfx::GFXDescriptorSet_sp             m_descriptorSet;
        gfx::GFXDescriptorSetLayout_sp       m_descriptorSetLayout;
        gfx::GFXBuffer_sp                    m_materialConstantBuffer;

        bool m_createdGpuResource = false;
        bool m_isDirtyParameter{};
        int  m_renderQueue{};

        hash_map<index_string, MaterialParameterValue> m_parameterValues;
    };

    DECL_PTR(Material);

} // namespace pulsar
