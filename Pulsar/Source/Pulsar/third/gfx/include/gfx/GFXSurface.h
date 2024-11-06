#pragma once
#include <cstdint>

namespace gfx
{
    class GFXSurface
    {
    public:
        virtual ~GFXSurface() = default;
        virtual intptr_t GetNativeHandle() const = 0;
        virtual void* GetUserPoint() const = 0;
        virtual void Initialize() = 0;
        virtual void CreateMainWindow() = 0;
        virtual int CreateVulkanSurface(void* instance, void* outSurface) { return -1; }
        virtual void DestroySurface() = 0;
        virtual void PollEvent() {}
        virtual void EnumEvents(void(*func)(void*)) {}
        virtual bool WantToClose() = 0;
    };

}
