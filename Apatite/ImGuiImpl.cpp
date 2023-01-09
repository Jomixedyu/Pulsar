#include "ImGuiImpl.h"
#include <Apatite/ImGuiImpl.h>

#include <ThirdParty/imgui/imgui.h>
#include <ThirdParty/imgui/imgui_impl_glfw.h>
#include <ThirdParty/imgui/imgui_impl_opengl3.h>

#include <Apatite/Private/SystemInterface.h>

namespace apatite
{
    using namespace detail;
    void ImGui_Engine_NewFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }
    void ImGui_Engine_EndFrame()
    {
        //ImGui::UpdatePlatformWindows();
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }
    void ImGui_Engine_Initialize()
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiWindowFlags_MenuBar;
        //io.ConfigFlags |= ImGuiWindowFlags_NoTitleBar;
        io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;
        
        io.Fonts->AddFontFromFileTTF("DroidSans.ttf", 18.0f);
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        
        //custom
        {
            style.FrameRounding = 12;
            style.FramePadding = { 12,5 };
            style.ItemSpacing = { 10,8 };
            style.ScrollbarSize = 18;
            style.Colors[ImGuiCol_::ImGuiCol_WindowBg] = ImColor(26, 26, 26, 255);
            style.Colors[ImGuiCol_::ImGuiCol_FrameBg] = ImColor(12, 12, 12, 255);
            //bug:
            //style.WindowMenuButtonPosition = ImGuiDir_::ImGuiDir_Right;
        }



        ImGui_ImplGlfw_InitForOpenGL((GLFWwindow*)SystemInterface::GetWindow(), true);
        ImGui_ImplOpenGL3_Init("#version 330");
    }
    void ImGui_Engine_Terminate()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }
}