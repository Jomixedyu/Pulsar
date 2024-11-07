#pragma once
#include "Texture2D.h"

namespace pulsar
{
    class SpriteInfo : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SpriteInfo, Object);
    public:
        CORELIB_REFL_DECL_FIELD(m_name);
        string m_name;
        CORELIB_REFL_DECL_FIELD(m_uv0);
        Vector2f m_uv0{};
        CORELIB_REFL_DECL_FIELD(m_uv1);
        Vector2f m_uv1{};
    };

    class SpriteAtlas : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SpriteAtlas, AssetObject);
        CORELIB_CLASS_ATTR(new CreateAssetAttribute)
    public:
        SpriteAtlas();

        static RCPtr<SpriteAtlas> StaticCreate(index_string name, RCPtr<Texture2D> tex, Vector2f uv0, Vector2f uv1);

        void PostEditChange(FieldInfo* info) override;

        RCPtr<Texture2D> GetTexture() const noexcept { return m_texture; }
        void             SetTexture(RCPtr<Texture2D> tex);

    protected:
        CORELIB_REFL_DECL_FIELD(m_sprites);
        List_sp<SPtr<SpriteInfo>> m_sprites;

        CORELIB_REFL_DECL_FIELD(m_texture);
        RCPtr<Texture2D> m_texture;
    };
    DECL_PTR(SpriteAtlas);


    class SpriteAtlasReference : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SpriteAtlasReference, Object);
    public:
        CORELIB_REFL_DECL_FIELD(m_atlas);
        RCPtr<SpriteAtlas> m_atlas;

        CORELIB_REFL_DECL_FIELD(m_index);
        int m_index{};
    };

}