#include "ImGuiImpl.h"
#include <Pulsar/ImGuiImpl.h>

#include <ThirdParty/imgui/imgui.h>
#include <ThirdParty/imgui/imgui_impl_glfw.h>
#include <ThirdParty/imgui/imgui_impl_opengl3.h>

#include <Pulsar/Private/SystemInterface.h>
#include <CoreLib.Platform/Window.h>

namespace pulsar
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
        
        io.ConfigWindowsMoveFromTitleBarOnly = true;

        io.Fonts->AddFontFromFileTTF("DroidSans.ttf", 14.4f);
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();
        
        //custom
        {
            style.WindowPadding = { 2.f, 2.f };
            style.WindowRounding = 8;
            style.FrameRounding = 6;
            style.FramePadding = { 12,3 };
            style.ItemSpacing = { 10,5 };
            style.ScrollbarSize = 10;
            
            style.Colors[ImGuiCol_::ImGuiCol_Button] = ImColor(58, 58, 58, 255);

            style.Colors[ImGuiCol_::ImGuiCol_WindowBg] = ImColor(26, 26, 26, 255);
            style.Colors[ImGuiCol_::ImGuiCol_FrameBg] = ImColor(12, 12, 12, 255);
            style.Colors[ImGuiCol_::ImGuiCol_TitleBg] = ImColor(25, 25, 28, 255);
            style.Colors[ImGuiCol_::ImGuiCol_TitleBgActive] = ImColor(60, 60, 60, 255);

            //tab
            style.Colors[ImGuiCol_::ImGuiCol_Tab] = ImColor(38, 38, 38, 255);
            style.Colors[ImGuiCol_::ImGuiCol_TabActive] = ImColor(255, 79, 79, 255);
            style.Colors[ImGuiCol_::ImGuiCol_TabHovered] = ImColor(255, 79, 79, 255);
            style.Colors[ImGuiCol_::ImGuiCol_TabUnfocused] = ImColor(38, 38, 38, 255);
            style.Colors[ImGuiCol_::ImGuiCol_TabUnfocusedActive] = ImColor(79, 79, 79, 255);

            style.Colors[ImGuiCol_::ImGuiCol_Header] = ImColor(48, 48, 48, 255);

            //bug:
            //style.WindowMenuButtonPosition = ImGuiDir_::ImGuiDir_Right;
            io.FontGlobalScale = jxcorlib::platform::window::GetUIScaling();
            io.MouseDrawCursor = true;
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