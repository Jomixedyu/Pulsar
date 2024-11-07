#include "Assets/SpriteAtlas.h"

namespace pulsar
{

    RCPtr<SpriteAtlas> SpriteAtlas::StaticCreate(index_string name, RCPtr<Texture2D> tex, Vector2f uv0, Vector2f uv1)
    {
        auto ptr = mksptr(new SpriteAtlas);
        ptr->Construct();

        ptr->SetIndexName(name);
        ptr->m_texture = std::move(tex);

        return ptr.get();
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
