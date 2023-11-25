#pragma once
#include <gfx/GFXApplication.h>
#include <gfx/GFXCommandBuffer.h>
#include <memory>
#include <CoreLib/UString.h>
#include <imgui/imgui.h>

namespace pulsar
{
    class ImGuiObject
    {
    public:
        virtual void Initialize() = 0;
        virtual void SetLayoutInfo(jxcorlib::string_view content) = 0;
        virtual void NewFrame() = 0;
        virtual void Render(gfx::GFXCommandBuffer* cmd) = 0;
        virtual void EndFrame() = 0;
        virtual void Terminate() = 0;
        virtual ~ImGuiObject() = default;
    };
    extern void ImGui_Style_Initialize();
    extern std::shared_ptr<ImGuiObject> CreateImGui(gfx::GFXApplication* app);

}