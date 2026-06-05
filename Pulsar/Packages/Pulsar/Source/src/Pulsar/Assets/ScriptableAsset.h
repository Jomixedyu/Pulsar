#pragma once
#include "Pulsar/AssetObject.h"

namespace pulsar
{
    class ScriptableAsset : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::ScriptableAsset, AssetObject);
    public:

        void Serialize(AssetSerializer* s) final;

    };
    DECL_PTR(ScriptableAsset);
} // namespace pulsar