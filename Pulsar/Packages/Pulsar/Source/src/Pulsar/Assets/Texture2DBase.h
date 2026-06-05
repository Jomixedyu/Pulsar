#pragma once
#include "Texture.h"

namespace pulsar
{
    class Texture2DBase : public Texture
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Texture2DBase, Texture);
    public:
        // Base class for all 2D textures (Texture2D, RenderTexture, CurveLinearColorAtlas).
        // TextureCube does NOT inherit from this.
    };
    DECL_PTR(Texture2DBase);
} // namespace pulsar
