#pragma once
#include "GFXBuffer.h"
#include "GFXTexture.h"
#include "GFXTextureView.h"

namespace gfx
{
    class GFXApplication;

    // Engine-wide, long-lived fallback GPU resources bound when a reflected
    // descriptor has no matching resource. Owned by GFXApplication; created
    // once the device is ready and released before the device is destroyed.
    class GFXBuiltinResources
    {
    public:
        // Fixed size of the shared zero buffer. It must cover the largest
        // uniform block; larger reflected UBO sizes assert at the binding site.
        static constexpr size_t ZeroBufferSize = 64 * 1024;

        void Initialize(GFXApplication* app);
        void Terminate();

        // Shared read-only zero-filled buffer. Bound as a UBO (range = reflected
        // size) or SSBO (whole size) when a buffer binding is missing.
        GFXBuffer* GetZeroBuffer() const { return m_zeroBuffer.get(); }

        // 2x2 opaque-black 2D texture view, bound when a 2D texture is missing.
        GFXTexture2DView* GetBlackTexture2DView() const { return m_black2DView.get(); }

    private:
        GFXApplication* m_app = nullptr;
        GFXBuffer_sp m_zeroBuffer;
        GFXTexture_sp m_black2D;
        GFXTexture2DView_sp m_black2DView;
    };
}
