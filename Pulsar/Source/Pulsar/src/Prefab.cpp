#include <Pulsar/Prefab.h>
#include <Pulsar/Node.h>

namespace pulsar
{

    RCPtr<Prefab> Prefab::StaticCreate(string_view name)
    {
        auto self = mksptr(new Prefab);
        self->Construct();
        self->SetName(name);

        return self;
    }
} // namespace pulsar