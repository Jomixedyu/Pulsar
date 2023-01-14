#pragma once
#include <Apatite/AssetObject.h>
#include <Apatite/IBindGPU.h>

namespace apatite
{
    class Texture : public AssetObject, public IBindGPU
    {
        CORELIB_DEF_TYPE(AssemblyObject_Apatite, apatite::Texture, AssetObject);

    public:
        uint32_t get_tex_id() const { return tex_id_; }
        virtual int32_t get_width() const = 0;
        virtual int32_t get_height() const = 0;
        virtual Vector2i GetSize2di() const { return { this->get_width(), this->get_height() }; }
        virtual Vector2f GetSize2df() const { return Vector2f(this->get_width(), this->get_height()); }
    protected:
        uint32_t tex_id_ = 0;
    };
    CORELIB_DECL_SHORTSPTR(Texture);
}