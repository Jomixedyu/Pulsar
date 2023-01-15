#pragma once

#include <Apatite/AssetObject.h>

namespace apatite
{

    
    class CubeMapAssetSourceImporterSetting : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::CubeMapAssetSourceImporterSetting, Object);
    public:
        CORELIB_REFL_DECL_FIELD(Top);
        guid_t Top;
        CORELIB_REFL_DECL_FIELD(Left);
        guid_t Left;
        CORELIB_REFL_DECL_FIELD(Bottom);
        guid_t Bottom;
        CORELIB_REFL_DECL_FIELD(Front);
        guid_t Front;
        CORELIB_REFL_DECL_FIELD(Back);
        guid_t Back;
        CORELIB_REFL_DECL_FIELD(Right);
        guid_t Right;
    };

    class CubeMapAsset : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::CubeMapAsset, AssetObject);
    public:
        //uint32_t id;
    };
    CORELIB_DECL_SHORTSPTR(CubeMapAsset);
}