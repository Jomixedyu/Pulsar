#pragma once
#include <Apatite/ObjectBase.h>
#include "IShadeParameters.h"

namespace apatite
{
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
    };

    class ShaderPass : public IShadeParameters
    {
    public:
        ShaderPass(const char* name, const char* vert_code, const char* frag_code);
        virtual ~ShaderPass();
        
    public:
        void EnableProgram();
        void DisableProgram();
    public:
        //IShaderParam
        virtual int32_t GetUniformLocaltion(string_view name) override;
        virtual void SetUniformInt(string_view name, const int32_t& i) override;
        virtual void SetUniformFloat(string_view name, const float& f) override;
        virtual void SetUniformMatrix4fv(string_view name, const float* value) override;
        virtual void SetUniformMatrix4fv(string_view name, const Matrix4f& mat) override;
        virtual void SetUniformVector3(string_view name, const Vector3f& value) override;
        virtual void SetUniformColor(string_view name, const LinearColorf& value) override;
        virtual void SetUniformColor(string_view name, const Vector3f& value) override;
    protected:
        uint32_t vert_id;
        uint32_t frag_id;
        uint32_t program_id;
        int32_t last_program_id;
    };

    struct ShaderPassScope
    {
        ShaderPass* program_;
        ShaderPassScope(ShaderPass* program) : program_(program)
        {
            program_->EnableProgram();
        }
        ~ShaderPassScope()
        {
            program_->DisableProgram();
        }
        ShaderPass* operator->()
        {
            return this->program_;
        }
    };
}