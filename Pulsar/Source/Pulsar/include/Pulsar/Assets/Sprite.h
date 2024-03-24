#pragma once
#include "Texture2D.h"

namespace pulsar
{
    class Sprite : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Sprite, AssetObject);
    public:
        static RCPtr<Sprite> StaticCreate(index_string name, Texture2D_ref tex, Vector2f uv0, Vector2f uv1);

        void PostEditChange(FieldInfo* info) override;

        Texture2D_ref GetTexture() const noexcept { return m_texture; }
        void          SetTexture(Texture2D_ref tex);
        Vector2f GetTexcoord0() const { return m_texcoord0; }
        void     SetTexcoord0(Vector2f value);
        Vector2f GetTexcoord1() const { return m_texcoord1; }
        void     SetTexcoord1(Vector2f value);
    protected:
        Vector2f m_texcoord0;
        Vector2f m_texcoord1;
        Texture2D_ref m_texture;
    };
    DECL_PTR(Sprite);

}