#include "Assets/Material.h"
#include <Pulsar/Assets/Material.h>

namespace pulsar
{

    Material_sp Material::StaticCreate(string_view name, Shader_rsp shader)
    {
        Material_sp material = mksptr(new Material);
        material->Construct();
        material->set_name(name);
        material->shader_ = shader;

        return material;
    }

}