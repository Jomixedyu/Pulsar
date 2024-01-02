#pragma once

#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Assets/Texture.h>
#include <variant>

namespace pulsar
{
    class IMaterialParameter
    {
    public:
        virtual void SetFloat(const index_string& name, float value) = 0;
        virtual void SetVector4(const index_string& name, const Vector4f& vec) = 0;
        virtual void SetTexture(const index_string& name, Texture_ref) = 0;

        virtual float GetFloat(const index_string& name) = 0;
        virtual Vector4f GetVector4(const index_string& name) = 0;
        virtual Texture_ref GetTexture(const index_string& name) = 0;
    };


    struct MaterialParameterValue
    {
    public:
        ShaderParameterType Type;

        void SetValue(int value)
        {
            Value = value;
            Type = ShaderParameterType::IntScalar;
        }

        void SetValue(float value)
        {
            Value = value;
            Type = ShaderParameterType::Scalar;;
        }

        void SetValue(Vector4f value)
        {
            Value = value;
            Type = ShaderParameterType::Vector;
        }

        void SetValue(Texture_ref value)
        {
            Value = value;
            Type = ShaderParameterType::Texture2D;
        }

    private:
        std::variant<int, float, Vector4f, Texture_ref> Value;

    public:
        int AsIntScalar() const
        {
            return std::get<int>(Value);
        }
        float AsScalar() const
        {
            return std::get<float>(Value);
        }
        Vector4f AsVector() const
        {
            return std::get<Vector4f>(Value);
        }
        Texture_ref AsTexture() const
        {
            return std::get<Texture_ref>(Value);
        }
        int GetDataSize() const
        {
            switch (Type)
            {
            case ShaderParameterType::IntScalar:
            case ShaderParameterType::Scalar:
                return sizeof(float);
            case ShaderParameterType::Vector:
                return sizeof(Vector4f);
            default:
                break;
            }
            return 0;
        }
    };

    struct MaterialParameterInfo
    {
        size_t Offset;
        MaterialParameterValue Value;
    };

    class Material final : public AssetObject, public IMaterialParameter, public IGPUResource
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Material, AssetObject);
        CORELIB_CLASS_ATTR(new MenuItemCreateAssetAttribute);

    public:
        static ObjectPtr<Material> StaticCreate(string_view name, Shader_ref shader);

        virtual void Serialize(AssetSerializer* s) override;

    public:
        virtual bool CreateGPUResource() override;
        virtual void DestroyGPUResource() override;
        virtual bool IsCreatedGPUResource() const override;

    public:
        void OnConstruct() override;

        // IMaterialParameter
        void SetIntScalar(const index_string& name, int value);
        void SetFloat(const index_string& name, float value) override;
        void SetTexture(const index_string& name, Texture_ref value) override;
        void SetVector4(const index_string& name, const Vector4f& value) override;
        int GetIntScalar(const index_string& name);
        float GetFloat(const index_string& name) override;
        Vector4f GetVector4(const index_string& name) override;
        Texture_ref GetTexture(const index_string& name) override;

        void SubmitParameters();

        gfx::GFXDescriptorSet_sp GetGfxDescriptorSet() const
        {
            return m_descriptorSet;
        }
        gfx::GFXDescriptorSetLayout_sp GetGfxDescriptorSetLayout() const
        {
            return m_descriptorSetLayout;
        }
        gfx::GFXShaderPass_sp GetGfxShaderPass();
    public:
        Shader_ref GetShader() const;
        void SetShader(Shader_ref value);

        ShaderPassRenderingType GetRenderingType() const;

        Action<> OnShaderChanged;

    public:
        const auto& GetShaderPropertyInfo() const { return m_propertyInfo; }
    protected:
        void PostEditChange(FieldInfo* info) override;

    private:
        CORELIB_REFL_DECL_FIELD(m_shader);
        Shader_ref m_shader;

        gfx::GFXShaderPass_sp m_gfxShaderPasses;


        std::vector<uint8_t> m_bufferData;

        gfx::GFXDescriptorSet_sp m_descriptorSet;
        gfx::GFXDescriptorSetLayout_sp m_descriptorSetLayout;
        gfx::GFXBuffer_sp m_materialConstantBuffer;
        size_t m_constantBufferSize{};

        bool m_createdGpuResource = false;
        bool m_isDirtyParameter{};
        int m_renderQueue{};

        ShaderPassConfig_sp m_shaderpassConfig;

        hash_map<index_string, MaterialParameterInfo> m_propertyInfo;

        hash_map<index_string, MaterialParameterInfo> m_parameterValues;
    };

    DECL_PTR(Material);

} // namespace pulsar
