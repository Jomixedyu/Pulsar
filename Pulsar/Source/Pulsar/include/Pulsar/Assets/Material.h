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
        virtual void SetInt(const string& name, int value) = 0;
        virtual void SetFloat(const string& name, float value) = 0;
        virtual void SetColor(const string& name, const Color4f& color) = 0;
        virtual void SetVector4(const string& name, const Vector4f& vec) = 0;
        virtual void SetTexture(const string& name, Texture_ref) = 0;
        virtual void SetMatrix4f(const string& name, const Matrix4f& value) = 0;

        virtual int         GetInt(const string& name) = 0;
        virtual float       GetFloat(const string& name) = 0;
        virtual Color4f     GetColor(const string& name) = 0;
        virtual Vector4f    GetVector4(const string& name) = 0;
        virtual Texture_ref GetTexture(const string& name) = 0;
        virtual Matrix4f    GetMatrix4f(const string& name) = 0;
    };

    struct MaterialParameterValue
    {
    public:
        enum { INT, FLOAT, COLOR, VECTOR, MATRIX, TEXTURE } Type;
        std::string Name;
        int Offset;
        void SetValue(int value) { Value = value; Type = INT; }
        void SetValue(float value) { Value = value; Type = FLOAT; }
        void SetValue(Color4f value) { Value = value; Type = COLOR; }
        void SetValue(Vector4f value) { Value = value; Type = VECTOR; }
        void SetValue(const Matrix4f& value) { Value = value; Type = MATRIX; }
        void SetValue(Texture_ref value) { Value = value; Type = TEXTURE; }
    private:
        std::variant<int, float, Color4f, Vector4f, Matrix4f, Texture_ref> Value;
    public:
        int AsInt() const { return std::get<int>(Value); }
        float AsFloat() const { return std::get<float>(Value); }
        Color4f AsColor() const { return std::get<Color4f>(Value); }
        Vector4f AsVector() const { return std::get<Vector4f>(Value); }
        Matrix4f AsMatrix() const { return std::get<Matrix4f>(Value); }
        Texture_ref AsTexture() const { return std::get<Texture_ref>(Value); }
        int GetDataSize() const
        {
            switch (Type)
            {
            case pulsar::MaterialParameterValue::INT: return sizeof(int);
                break;
            case pulsar::MaterialParameterValue::FLOAT: return sizeof(float);
                break;
            case pulsar::MaterialParameterValue::COLOR: return sizeof(Color4f);
                break;
            case pulsar::MaterialParameterValue::VECTOR: return sizeof(Vector4f);
                break;
            case pulsar::MaterialParameterValue::MATRIX: return sizeof(Matrix4f);
                break;
            }
            return 0;
        }

    };


    class Material : public AssetObject, public IMaterialParameter, public IGPUResource
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Material, AssetObject);
    public:
        static ObjectPtr<Material> StaticCreate(string_view name, Shader_ref shader);

        virtual void Serialize(AssetSerializer* s) override;
    public:
        virtual void CreateGPUResource() override;
        virtual void DestroyGPUResource() override;
        virtual bool IsCreatedGPUResource() const override;
    public:

        virtual void OnConstruct() override;
        void BeginGpu();
        void EndGpu();

        // IMaterialParameter
        virtual void SetInt(const string& name, int value) override;
        virtual void SetFloat(const string& name, float value) override;
        virtual void SetColor(const string& name, const Color4f& color) override;
        virtual void SetTexture(const string& name, Texture_ref value) override;
        virtual void SetMatrix4f(const string& name, const Matrix4f& value) override;
        virtual void SetVector4(const string& name, const Vector4f& value) override;
        virtual int         GetInt(const string& name) override;
        virtual float       GetFloat(const string& name) override;
        virtual Color4f     GetColor(const string& name) override;
        virtual Vector4f    GetVector4(const string& name) override;
        virtual Texture_ref GetTexture(const string& name) override;
        virtual Matrix4f    GetMatrix4f(const string& name) override;


        void CommitParameters();

        gfx::GFXDescriptorSet_sp GetDescriptorSet() const { return m_descriptorSet; }

    public:
        Shader_ref GetShader() const;
        void SetShader(Shader_ref value) { m_shader = value; }
    private:

        CORELIB_REFL_DECL_FIELD(m_shader);
        Shader_ref m_shader;

        hash_map<string, MaterialParameterValue> m_parameterValues;
        std::vector<uint8_t> m_bufferData;

        gfx::GFXDescriptorSet_sp m_descriptorSet;
        gfx::GFXDescriptorSetLayout_sp m_descriptorSetLayout;
        gfx::GFXBuffer_sp m_buffer;

        bool m_createdGpuResource = false;
        bool m_isDirtyParameter;
        int m_renderQueue;
    };
    DECL_PTR(Material);


}
