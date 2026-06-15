#pragma once
#include <gfx/GFXApplication.h>
#include <gfx/GFXCommandBuffer.h>
#include <memory>
#include <vector>
#include <CoreLib/UString.h>
#include <imgui/imgui.h>

namespace pulsar
{
    // Deep copy of an ImGui frame's draw data, owned independently of the global
    // ImGui context. Built on the main thread (after ImGui::Render) and consumed on
    // the render thread, so the next NewFrame can overwrite the context freely.
    class ImGuiDrawDataSnapshot
    {
    public:
        ImGuiDrawDataSnapshot() = default;
        ~ImGuiDrawDataSnapshot() { Clear(); }

        ImGuiDrawDataSnapshot(const ImGuiDrawDataSnapshot&) = delete;
        ImGuiDrawDataSnapshot& operator=(const ImGuiDrawDataSnapshot&) = delete;

        // Deep-clone src into this snapshot (replaces previous content).
        void CopyFrom(ImDrawData* src);
        void Clear();

        // Valid after CopyFrom; points at this snapshot's owned draw lists.
        ImDrawData* GetDrawData() { return m_valid ? &m_drawData : nullptr; }

    private:
        ImDrawData               m_drawData{};
        std::vector<ImDrawList*> m_clonedLists; // owned; freed in Clear()
        bool                     m_valid = false;
    };

    class ImGuiObject
    {
    public:
        virtual void Initialize() = 0;
        virtual void SetLayoutInfo(jxcorlib::string_view content) = 0;
        virtual std::string GetLayoutInfo() = 0;
        virtual void NewFrame() = 0;
        // [Main thread] Finalize the ImGui frame (ImGui::Render) and deep-copy the
        // draw data into the snapshot for the render thread to consume.
        virtual void PrepareDrawData(ImGuiDrawDataSnapshot& snapshot) = 0;
        // [Render thread] Issue GPU draw commands from a previously prepared snapshot.
        virtual void Render(gfx::GFXCommandBuffer* cmd, ImGuiDrawDataSnapshot& snapshot) = 0;
        virtual void EndFrame() = 0;
        virtual void Terminate() = 0;
        virtual bool IsMinimized() const { return false; }
        virtual ~ImGuiObject() = default;
    };
    extern void ImGui_Style_Initialize();
    extern std::shared_ptr<ImGuiObject> CreateImGui(gfx::GFXApplication* app);

}