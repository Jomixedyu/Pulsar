#include "Assets/Shader.h"

#include <Pulsar/Assets/Shader.h>
#include <CoreLib.Serialization/JsonSerializer.h>

#include "AssetManager.h"
#include "BuiltinAsset.h"
#include <Pulsar/Logger.h>

namespace pulsar
{
    using namespace std;

    RCPtr<Shader> Shader::StaticCreate(string_view name)
    {
        auto self = NewAssetObject<Shader>();
        self->SetName(name);

        return self;
    }

    Shader::Shader()
    {
        init_sptr_member(m_config);
    }

    void Shader::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);

        if (!s->IsWrite) // read
        {
            m_config = ser::JsonSerializer::Deserialize<ShaderConfig>(s->Object->ToString());
        }


    }

    void Shader::OnDestroy()
    {
        base::OnDestroy();
    }

}
