#pragma once

#include <Pulsar/AssetObject.h>

namespace pulsar
{
    class DataAsset : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::DataAsset, AssetObject);

    public:
        void Serialize(AssetSerializer* s) final;
    };
    DECL_PTR(DataAsset);
} // namespace pulsar
