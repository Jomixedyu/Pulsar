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
        virtual void SetInt(const index_string& name, int value) = 0;
        virtual void SetFloat(const index_string& name, float value) = 0;
        virtual void SetColor(const index_string& name, const Color4f& color) = 0;
        virtual void SetVector4(const index_string& name, const Vector4f& vec) = 0;
        virtual void SetTexture(const index_string& name, Texture_ref) = 0;
        virtual void SetMatrix4f(const index_string& name, const Matrix4f& value) = 0;

        virtual int GetInt(const index_string& name) = 0;
        virtual float GetFloat(const index_string& name) = 0;
        virtual Color4f GetColor(const index_string& name) = 0;
        virtual Vector4f GetVector4(const index_string& name) = 0;
        virtual Texture_ref GetTexture(const index_string& name) = 0;
        virtual Matrix4f GetMatrix4f(const index_string& name) = 0;
    };


    struct MaterialParameterValue
    {
    public:
        enum
        {
            INT,
            FLOAT,
            COLOR,
            VECTOR,
            MATRIX,
            TEXTURE
        } Type;
        std::string Name;
        int Offset;
        void SetValue(int value)
        {
            Value = value;
            Type = INT;
        }
        void SetValue(float value)
        {
            Value = value;
            Type = FLOAT;
        }
        void SetValue(Color4f value)
        {
            Value = value;
            Type = COLOR;
        }
        void SetValue(Vector4f value)
        {
            Value = value;
            Type = VECTOR;
        }
        void SetValue(const Matrix4f& value)
        {
            Value = value;
            Type = MATRIX;
        }
        void SetValue(Texture_ref value)
        {
            Value = value;
            Type = TEXTURE;
        }

    private:
        std::variant<int, float, Color4f, Vector4f, Matrix4f, Texture_ref> Value;

    public:
        int AsInt() const
        {
            return std::get<int>(Value);
        }
        float AsFloat() const
        {
            return std::get<float>(Value);
        }
        Color4f AsColor() const
        {
            return std::get<Color4f>(Value);
        }
        Vector4f AsVector() const
        {
            return std::get<Vector4f>(Value);
        }
        Matrix4f AsMatrix() const
        {
            return std::get<Matrix4f>(Value);
        }
        Texture_ref AsTexture() const
        {
            return std::get<Texture_ref>(Value);
        }
        int GetDataSize() const
        {
            switch (Type)
            {
            case INT:
                return sizeof(int);
            case FLOAT:
                return sizeof(float);
            case COLOR:
                return sizeof(Color4f);
            case VECTOR:
                return sizeof(Vector4f);
            case MATRIX:
                return sizeof(Matrix4f);
            default:
                break;
            }
            return 0;
        }
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
        virtual void OnConstruct() override;

        // IMaterialParameter
        virtual void SetInt(const index_string& name, int value) override;
        virtual void SetFloat(const index_string& name, float value) override;
        virtual void SetColor(const index_string& name, const Color4f& color) override;
        virtual void SetTexture(const index_string& name, Texture_ref value) override;
        virtual void SetMatrix4f(const index_string& name, const Matrix4f& value) override;
        virtual void SetVector4(const index_string& name, const Vector4f& value) override;
        virtual int GetInt(const index_string& name) override;
        virtual float GetFloat(const index_string& name) override;
        virtual Color4f GetColor(const index_string& name) override;
        virtual Vector4f GetVector4(const index_string& name) override;
        virtual Texture_ref GetTexture(const index_string& name) override;
        virtual Matrix4f GetMatrix4f(const index_string& name) override;

        void CommitParameters();

        gfx::GFXDescriptorSet_sp GetGfxDescriptorSet() const
        {
            return m_descriptorSet;
        }

    public:
        Shader_ref GetShader() const;
        void SetShader(Shader_ref value);

        size_t GetShaderPassCount() const
        {
            return m_shader->GetPassCount();
        }
        gfx::GFXShaderPass_sp GetGfxShaderPass(size_t index);

        Action<> OnShaderChanged;
    protected:
        void PostEditChange(FieldInfo* info) override;

    private:
        CORELIB_REFL_DECL_FIELD(m_shader);
        Shader_ref m_shader;

        array_list<gfx::GFXShaderPass_sp> m_gfxShaderPasses;

        hash_map<index_string, MaterialParameterValue> m_parameterValues;
        std::vector<uint8_t> m_bufferData;

        gfx::GFXDescriptorSet_sp m_descriptorSet;
        gfx::GFXDescriptorSetLayout_sp m_descriptorSetLayout;
        gfx::GFXBuffer_sp m_materialBuffer;

        bool m_createdGpuResource = false;
        bool m_isDirtyParameter{};
        int m_renderQueue{};
    };

    DECL_PTR(Material);

} // namespace pulsar
