#include "Assets/Sprite.h"

namespace pulsar
{

    RCPtr<Sprite> Sprite::StaticCreate(index_string name, Texture2D_ref tex, Vector2f uv0, Vector2f uv1)
    {
        auto ptr = mksptr(new Sprite);
        ptr->Construct();

        ptr->SetIndexName(name);
        ptr->m_texture = tex;
        ptr->m_texcoord0 = uv0;
        ptr->m_texcoord1 = uv1;

        return ptr.get();
    }
    void Sprite::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
    }
    void Sprite::SetTexture(Texture2D_ref tex)
    {
        m_texture = tex;
    }
    void Sprite::SetTexcoord0(Vector2f value)
    {
        m_texcoord0 = value;
    }
    void Sprite::SetTexcoord1(Vector2f value)
    {
        m_texcoord1 = value;
    }

} // namespace pulsar
