#include <Pulsar/Prefab.h>
#include <Pulsar/Node.h>

namespace pulsar
{

    RCPtr<Prefab> Prefab::StaticCreate(string_view name)
    {
        auto self = NewAssetObject<Prefab>();
        self->SetName(name);

        return self;
    }
} // namespace pulsar