#pragma once

#include <Apatite/ObjectBase.h>
#include <Apatite/AssetObject.h>
#include <Apatite/Assets/Texture.h>
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

    class ShaderPass
    {
        string name;
        ShaderCullMode cull;
        ShaderBlendMode blend;
        ShaderZTest ztest;
        bool zwrite;
        string vert_code;
        string frag_code;
    };

    class Shader : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::Shader, AssetObject);
    
    public:
        uint32_t get_id() const { return this->id_; }
        bool get_isused() const { return this->id_ == current_use_id; }
    public:
        explicit Shader();
        virtual ~Shader() override;
        virtual string ToString() const override;
    public:
        void UseProgram();
        void AttachShader(const Shader& shaderId);
        void Link();

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
        //static Shader_sp StatiCreate(const string& vert_code, const string&);
    protected:
        string name_;
        uint32_t id_;
        uint32_t vert_;
        uint32_t frag_;

        string vert_code_;
        string frag_code;
    private:
        inline static uint32_t current_use_id = 0;
    };
    CORELIB_DECL_SHORTSPTR(Shader);


    class ShaderCompileException : public EngineException
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::ShaderCompileException, EngineException);
    public:
        ShaderCompileException(const string& name, const string& msg) : base(msg), name_(name)
        {
            this->message_.insert(0, "filename: " + name + ", ");
        }

        string name_;
    };


}
