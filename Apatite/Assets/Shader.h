#pragma once

#include <Apatite/ObjectBase.h>
#include <Apatite/AssetObject.h>
#include <Apatite/Assets/Texture.h>
#include <Apatite/IBindGPU.h>
#include <Apatite/Math.h>

namespace apatite
{
    class Texture2D;

    enum class ShaderCullMode
    {
        CullOff,
        CullBack,
        CullFront,
    };
    enum class ShaderZTest
    {
        Always,
        NotEqual,
        Less,
        LEqual,
        Equal,
        GEqual,
        Greater
    };
    enum class ShaderBlendMode
    {
        None,
        SrcAlpha_OneMinusSrcAlpha
    };


    struct ShaderPassConfig
    {
        string name;
        ShaderCullMode cull;
        ShaderBlendMode blend;
        ShaderZTest ztest;
        bool zwrite;
        string vert_code;
        string frag_code;
    };

    struct ShaderPass
    {

    public:
        ShaderPassConfig config_;
        uint32_t vert;
        uint32_t frag;
        uint32_t program;
    };


    class Shader : public AssetObject, public IBindGPU
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::Shader, AssetObject);
    public:
        uint32_t get_id() const { return this->id_; }
    public:
        explicit Shader();
        virtual ~Shader() override;
        virtual string ToString() const override;
    public:
        void UseShader();
    public:
        virtual void BindGPU() override;
        virtual void UnBindGPU() override;
        virtual bool GetIsBindGPU() override;
    public:
        int32_t GetUniformLocaltion(std::string_view name);
        void SetUniformInt(std::string_view name, const int32_t& i);
        void SetUniformFloat(std::string_view name, const float& f);
        void SetUniformMatrix4fv(std::string_view name, const float* value);
        void SetUniformMatrix4fv(std::string_view name, const Matrix4f& mat);
        void SetUniformVector3(std::string_view name, const Vector3f& value);
        void SetUniformColor(std::string_view name, const LinearColorf& value);
        void SetUniformColor(std::string_view name, const Vector3f& value);

        void SetUniformTexture(std::string_view name, Texture_rsp tex);
    protected:
    public:
        static sptr<Shader> StaticCreate(string_view name, array_list<ShaderPass>&& pass);
    protected:
        string name_;
        uint32_t id_;
        array_list<ShaderPass> pass_;
    private:
    };
    CORELIB_DECL_SHORTSPTR(Shader);


    class ShaderCompileException : public EngineException
    {
        
    public:
        virtual const char* name() const override { return "ShaderCompileException"; }

        ShaderCompileException(const string& name, const string& msg) : EngineException(msg), name_(name)
        {
            this->message_.insert(0, "filename: " + name + "");
        }

        string name_;
    };


}
