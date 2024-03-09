#pragma once
#include <PulsarEd/Assembly.h>
#include <Pulsar/AssetObject.h>


namespace pulsared
{


    class AssetUtil
    {
    public:
        static void OpenAssetEditor(RCPtr<AssetObject> asset);
    };
}