#include "Pulsar/Assets/ScriptableAsset.h"
#include "CoreLib.Serialization/JsonSerializer.h"

namespace pulsar
{

    void ScriptableAsset::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        if (s->IsWrite)
        {
            auto json = ser::JsonSerializer::Serialize(this, {});
            s->Object->AssignParse(json);
        }
        else // read
        {
            ser::JsonSerializer::Deserialize(s->Object->ToString(false), GetType(), self());
        }
    }
} // namespace pulsar