#pragma once
#include "ObjectBase.h"
#include <CoreLib/Guid.h>
#include <Apatite/AssetObject.h>

namespace apatite
{
    struct AssetRegisterInfo
    {
        string path;
        std::function<AssetObject_sp(guid_t)> getter;
    };

    class AssetRegistry
    {
    public:
        static bool IsLoadable(guid_t guid);
        static AssetObject_sp FindAsset(guid_t guid);
        static AssetObject_sp FindAssetAtPath(string_view path);
        static void Reset();
        static void RegisterAsset(guid_t guid, AssetRegisterInfo info);
    };

}