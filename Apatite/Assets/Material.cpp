#include <Apatite/Assets/Material.h>
#include <ThirdParty/glad/glad.h>

namespace apatite
{
    void Material::SetFloat(string_view name, float f)
    {
        this->shader_->SetUniformFloat(name, f);
    }

    void Material::SetVector3(string_view name, Vector3f vec)
    {
        this->shader_->SetUniformVector3(name, vec);
    }
    void Material::SetTexture(string_view name, Texture_rsp tex)
    {
        this->shader_->SetUniformTexture(name, tex);
    }

    void Material::UseMaterial()
    {
        this->shader_->UseShader();
    }

}