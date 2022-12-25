#pragma once
#include <Apatite/AssetObject.h>


namespace apatite
{
    class Texture : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::Texture, AssetObject);

    public:
        uint32_t get_tex_id() const { return tex_id_; }

    protected:
        uint32_t tex_id_ = 0;
    };
    CORELIB_DECL_SHORTSPTR(Texture);
}