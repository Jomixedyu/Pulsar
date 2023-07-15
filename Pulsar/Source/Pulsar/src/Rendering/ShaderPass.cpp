#include "Rendering/ShaderPass.h"
#include <Pulsar/Assets/Shader.h>

namespace pulsar
{

    ShaderPass::ShaderPass(const char* name, const char* vert_code, const char* frag_code)
    {
      
    }

    int32_t ShaderPass::GetUniformLocaltion(string_view name)
    {
        return 0;
    }
    void ShaderPass::SetUniformInt(string_view name, const int32_t& i)
    {
        
    }
    void ShaderPass::SetUniformFloat(string_view name, const float& f)
    {

    }
    void ShaderPass::SetUniformMatrix4fv(string_view name, const float* value)
    {

    }
    void ShaderPass::SetUniformMatrix4fv(string_view name, const Matrix4f& mat)
    {
        SetUniformMatrix4fv(name, mat.get_value_ptr());
    }
    void ShaderPass::SetUniformVector3(string_view name, const Vector3f& value)
    {

    }
    void ShaderPass::SetUniformColor(string_view name, const LinearColorf& value)
    {

    }
    void ShaderPass::SetUniformColor(string_view name, const Vector3f& value)
    {
        SetUniformColor(name, LinearColorf(value.x, value.y, value.z));
    }

    ShaderPass::~ShaderPass()
    {

    }

    void ShaderPass::EnableProgram()
    {

    }
    void ShaderPass::DisableProgram()
    {

    }
}