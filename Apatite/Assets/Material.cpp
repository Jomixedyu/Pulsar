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

    //void Material::BindGPU()
    //{
    //    this->shader_->UseProgram();

    //    int32_t i = 0;
    //    for (auto& [name, tex] : this->textures)
    //    {
    //        glActiveTexture(GL_TEXTURE1 + i);
    //        glBindTexture(GL_TEXTURE_2D, tex->get_id());
    //        this->program->SetUniformInt(name, tex->get_id());
    //        i++;
    //    }

    //    glActiveTexture(GL_TEXTURE0);

    //}

}