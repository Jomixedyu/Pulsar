#pragma once

#include <Pulsar/AssetObject.h>
#include <Pulsar/Assets/Shader.h>
#include <Pulsar/Assets/Texture.h>

namespace pulsar
{

    class Material : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_Pulsar, pulsar::Material, AssetObject);
        friend struct MaterialScope;
    public:

        Shader_sp GetShader() const { return this->shader_; }

        static sptr<Material> StaticCreate(string_view name, Shader_rsp shader);
    private:
        Shader_sp shader_;
        int m_renderQueue;
    };
    DECL_PTR(Material);


}
