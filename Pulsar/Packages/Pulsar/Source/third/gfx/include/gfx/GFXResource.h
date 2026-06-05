#pragma once
#include "GFXResourceType.h"
#include "GFXInclude.h"

namespace gfx
{
    // Base class for all GPU resources.
    // Provides a common root and resource type identification.
    class GFXResource
    {
    public:
        virtual ~GFXResource() = default;
        virtual GFXResourceType GetResourceType() const = 0;

        // Internal resource ID assigned by the resource manager.
        uint32_t GetResourceId() const { return m_resourceId; }
        void SetResourceId(uint32_t id) { m_resourceId = id; }

    private:
        uint32_t m_resourceId = UINT32_MAX;
    };
    GFX_DECL_SPTR(GFXResource);

} // namespace gfx
