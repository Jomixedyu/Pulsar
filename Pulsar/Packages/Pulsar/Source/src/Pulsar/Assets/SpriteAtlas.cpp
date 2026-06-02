#include "Assets/SpriteAtlas.h"

namespace pulsar
{

    SpriteAtlas::SpriteAtlas()
    {
        init_sptr_member(m_sprites);
    }

    RCPtr<SpriteAtlas> SpriteAtlas::StaticCreate(index_string name, RCPtr<Texture2D> tex, Vector2f uv0, Vector2f uv1)
    {
        auto ptr = NewAssetObject<ThisClass>();

        ptr->SetIndexName(name);
        ptr->m_texture = std::move(tex);

        return ptr;
    }

    void SpriteAtlas::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        if (!info)
        {
            return;
        }
        if (info->GetName() == NAMEOF(m_texture))
        {
            SetTexture(m_texture);
        }
    }

    void SpriteAtlas::SetTexture(RCPtr<Texture2D> tex)
    {
        m_texture = std::move(tex);

    }


} // namespace pulsar
