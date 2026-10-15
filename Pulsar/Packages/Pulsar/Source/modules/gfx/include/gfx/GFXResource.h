#pragma once
#include "GFXResourceType.h"
#include "GFXInclude.h"
#include <atomic>
#if !defined(NDEBUG)
#include <string>
#endif

namespace gfx
{
    class GFXResourceRegistry;
}

namespace gfx
{
    // Base class for all GPU resources.
    // Provides a common root and resource type identification.
    class GFXResource
    {
    public:
        virtual ~GFXResource() = default;
        GFXResource(const GFXResource&) = delete;
        GFXResource& operator=(const GFXResource&) = delete;
        virtual GFXResourceType GetResourceType() const = 0;

        using ResourceId = uint64_t;
        static constexpr ResourceId InvalidResourceId = 0;

        // Process-unique id generated once when the resource object is constructed.
        ResourceId GetResourceId() const { return m_resourceId; }

        GFXResourceRegistry* GetResourceRegistry() const { return m_registry; }

#if !defined(NDEBUG)
        // Debug-only creation site info (a few stack frames as text),
        // captured by the resource registry at creation time.
        const std::string& GetCreationDebugInfo() const { return m_creationDebugInfo; }
        void SetCreationDebugInfo(std::string info) { m_creationDebugInfo = std::move(info); }
#endif

    protected:
        explicit GFXResource(GFXResourceRegistry* registry = nullptr);

    private:
        ResourceId m_resourceId = InvalidResourceId;
        GFXResourceRegistry* m_registry = nullptr;
#if !defined(NDEBUG)
        std::string m_creationDebugInfo;
#endif
    };
    GFX_DECL_PTR(GFXResource);

} // namespace gfx
