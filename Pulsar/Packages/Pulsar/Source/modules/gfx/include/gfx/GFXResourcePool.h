#pragma once
#include "GFXBuffer.h"
#include "GFXTexture.h"

namespace gfx
{
    class GFXApplication;

    class GFXResourcePool
    {
    public:
        explicit GFXResourcePool(GFXApplication* app) : m_app(app) {}

        std::shared_ptr<GFXTexture> UseTexture(const GFXTextureCreateDesc& desc);
        std::shared_ptr<GFXBuffer> UseBuffer(const GFXBufferDesc& desc);


        void Update();
    protected:
        GFXApplication* m_app;
    };
} // namespace gfx