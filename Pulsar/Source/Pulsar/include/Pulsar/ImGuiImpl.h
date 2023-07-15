#pragma once
#include <gfx/GFXApplication.h>
#include <gfx/GFXCommandBuffer.h>
#include <memory>

namespace pulsar
{
    class ImGuiObject
    {
    public:
        virtual void Initialize() = 0;
        virtual void NewFrame() = 0;
        virtual void Render(gfx::GFXCommandBuffer* cmd) = 0;
        virtual void EndFrame() = 0;
        virtual void Terminate() = 0;
    };
    extern void ImGui_Style_Initialize();
    extern std::shared_ptr<ImGuiObject> CreateImGui(gfx::GFXApplication* app);

}