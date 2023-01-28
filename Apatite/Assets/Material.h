#pragma once

#include <Apatite/AssetObject.h>
#include <Apatite/Math.h>
#include <Apatite/Assets/Shader.h>
#include <Apatite/Assets/Texture.h>

namespace apatite
{

    class Material : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::Material, AssetObject);
        friend struct MaterialScope;
    public:
        void SetTexture(string_view name, Texture_rsp value);
        void SetFloat(string_view name, float f);
        void SetVector3(string_view name, Vector3f vec);

        void set_shader(Shader_sp shader) { this->shader_ = shader; }
        Shader_sp get_shader() const { return this->shader_; }

        static sptr<Material> StaticCreate(string_view name, Shader_rsp shader);
    private:
        Shader_sp shader_;
    };
    CORELIB_DECL_SHORTSPTR(Material);


}
