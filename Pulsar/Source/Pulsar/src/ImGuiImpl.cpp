#include "ImGuiImpl.h"
#include <Pulsar/ImGuiImpl.h>
#include <imgui/imgui.h>
#include <imgui/imgui_impl_sdl2.h>

#include "DroidSans.ttf.h"
#include "forkawesome-webfont.ttf.h"
#include "IconsForkAwesome.h"

#include <CoreLib.Platform/Window.h>
#include <gfx/GFXApplication.h>

#if PULSAR_GFX_BUILD_VULKAN
#include <imgui/imgui_impl_vulkan.h>
#include <imgui/imgui_impl_sdl2.h>
#include <gfx-vk/GFXVulkanApplication.h>
#include <gfx-vk/GFXVulkanDescriptorManager.h>
#include <gfx-vk/GFXVulkanCommandBuffer.h>
#endif
#include <imgui/imgui_impl_win32.h>

namespace pulsar
{



    void ImGui_Style_Initialize()
    {
        //ImGui::StyleColorsDark();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.IniFilename = nullptr;

        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiWindowFlags_MenuBar;
        //io.ConfigFlags |= ImGuiConfigFlags_IsSRGB;
        //io.ConfigFlags |= ImGuiWindowFlags_NoTitleBar;
        io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;

        io.ConfigWindowsMoveFromTitleBarOnly = true;
        //io.MouseDrawCursor = true;

        {
            ImFontConfig fontConfig{};
            fontConfig.FontDataOwnedByAtlas = false;
            fontConfig.MergeMode = false;
            fontConfig.OversampleH = 2;
            fontConfig.OversampleV = 2;
            fontConfig.GlyphExtraSpacing.x = 0.5f;
            strcpy(fontConfig.Name, "ForkAwesome");
            static const ImWchar icon_ranges[] = { ICON_MIN_FK, ICON_MAX_FK, 0 };
            io.Fonts->AddFontFromMemoryTTF(FILE_forkawesome_webfont_ttf, sizeof(FILE_forkawesome_webfont_ttf), 13, &fontConfig, icon_ranges);
        }
        {
            ImFontConfig fontConfig{};
            fontConfig.FontDataOwnedByAtlas = false;
            fontConfig.MergeMode = true;
            fontConfig.OversampleH = 2;
            fontConfig.OversampleV = 2;
            fontConfig.GlyphExtraSpacing.x = 0.5f;
            strcpy(fontConfig.Name, "DroidSans");
            io.Fonts->AddFontFromMemoryTTF(FILE_DroidSans_ttf, sizeof(FILE_DroidSans_ttf), 14.f, &fontConfig);
        }

        {
            ImFontConfig fontConfig{};
            fontConfig.FontDataOwnedByAtlas = true;
            fontConfig.MergeMode = true;
            //fontConfig.OversampleH = 2;
            //fontConfig.OversampleV = 2;
            //strcpy(fontConfig.Name, "YaHei");
            io.Fonts->AddFontFromFileTTF("C:\\Windows\\Fonts\\msyh.ttc", 16.f, &fontConfig, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
        }

       

        io.FontGlobalScale = jxcorlib::platform::window::GetUIScaling();
        //io.Fonts->AddFontFromFileTTF("DroidSans.ttf", 14.4f);
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();

        //custom

        style.WindowPadding = { 2.f, 2.f };
        style.WindowRounding = 4;
        style.FrameRounding = 4;
        style.FramePadding = { 8,3 };
        style.ItemSpacing = { 10,5 };
        style.ScrollbarSize = 10;
        style.ItemInnerSpacing = { 10,4 };
        style.GrabMinSize = 14;
        style.GrabRounding = 12;
        style.WindowTitleAlign = { 0.5,0.5 };
        style.SeparatorTextBorderSize = 1;
        style.SeparatorTextAlign = { 0.5,0.5 };
        style.SeparatorTextPadding = { 0,0 };
        style.DockingSeparatorSize = 3;


        style.Colors[ImGuiCol_Text] = ImColor(214, 214, 214, 255);
        style.Colors[ImGuiCol_TextDisabled] = ImColor(110, 110, 110, 255);
        style.Colors[ImGuiCol_WindowBg] = ImColor(38, 38, 38, 255);
        style.Colors[ImGuiCol_ChildBg] = ImColor(30, 30, 30, 128);
        style.Colors[ImGuiCol_Border] = ImColor(140, 136, 138, 79);
        style.Colors[ImGuiCol_FrameBg] = ImColor(55, 55, 55, 255);
        style.Colors[ImGuiCol_FrameBgHovered] = ImColor(97, 96, 148, 102);
        style.Colors[ImGuiCol_FrameBgActive] = ImColor(53, 46, 68, 172);
        style.Colors[ImGuiCol_TitleBg] = ImColor(28, 28, 28, 255);
        style.Colors[ImGuiCol_TitleBgActive] = ImColor(50, 50, 50, 255);
        style.Colors[ImGuiCol_MenuBarBg] = ImColor(36, 36, 36, 255);
        style.Colors[ImGuiCol_CheckMark] = ImColor(128, 99, 244, 255);
        style.Colors[ImGuiCol_SliderGrab] = ImColor(120, 90, 205, 255);
        style.Colors[ImGuiCol_SliderGrabActive] = ImColor(118, 66, 255, 255);
        style.Colors[ImGuiCol_Button] = ImColor(66, 66, 66, 255);
        style.Colors[ImGuiCol_ButtonHovered] = ImColor(64, 53, 130, 255);
        style.Colors[ImGuiCol_ButtonActive] = ImColor(145, 131, 238, 255);
        style.Colors[ImGuiCol_Header] = ImColor(101, 74, 169, 255);
        style.Colors[ImGuiCol_HeaderHovered] = ImColor(66, 150, 126, 204);
        style.Colors[ImGuiCol_HeaderActive] = ImColor(66, 150, 156, 255);
        style.Colors[ImGuiCol_Tab] = ImColor(28, 28, 28, 0);
        style.Colors[ImGuiCol_TabUnfocused] = ImColor(28, 28, 28, 0);
        style.Colors[ImGuiCol_TabUnfocusedActive] = ImColor(38, 38, 38, 255);
        style.Colors[ImGuiCol_TabHovered] = ImColor(113, 96, 232, 255);
        style.Colors[ImGuiCol_TabActive] = ImColor(113, 96, 232, 255);
        style.Colors[ImGuiCol_TextSelectedBg] = ImColor(141, 98, 254, 255);
        style.Colors[ImGuiCol_Border] = ImColor(28, 28, 28, 255);

    }



#if PULSAR_GFX_BUILD_VULKAN
    class ImGuiImpl_Vulkan final : public ImGuiObject
    {
    public:
        ImGuiImpl_Vulkan(gfx::GFXVulkanApplication* app)
            : m_app(app)
        { }


        gfx::GFXVulkanApplication* m_app;

        virtual void Initialize() override
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            //ImGuiIO& io = ImGui::GetIO(); (void)io;
            //ImGui::StyleColorsClassic();
            //ImGui::StyleColorsDark();
            ImGui_Style_Initialize();

            ImGui_ImplSDL2_InitForVulkan((SDL_Window*)m_app->GetWindow()->GetUserPoint());

            ImGui_ImplVulkan_InitInfo init_info = {};
            init_info.Instance = m_app->GetVkInstance();
            init_info.PhysicalDevice = m_app->GetVkPhysicalDevice();
            init_info.Device = m_app->GetVkDevice();
            init_info.QueueFamily = 0;
            init_info.Queue = m_app->GetVkGraphicsQueue();
            init_info.PipelineCache = VK_NULL_HANDLE;
            init_info.MinImageCount = 2;
            init_info.ImageCount = 2;
            init_info.DescriptorPool = m_app->GetVulkanDescriptorManager()->GetCommonDescriptorSetPool()->GetVkDescriptorPool();
            init_info.Allocator = VK_NULL_HANDLE;

            ImGui_ImplVulkan_Init(&init_info, m_app->GetVulkanViewport()->GetRenderPass()->GetVkRenderPass());

            {
                gfx::GFXVulkanCommandBuffer font(m_app);
                VkCommandBufferBeginInfo beginInfo{};
                {
                    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
                    beginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
                }
                font.Begin();
                ImGui_ImplVulkan_CreateFontsTexture(font.GetVkCommandBuffer());
                font.End();
                VkSubmitInfo submitInfo{};
                {
                    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
                    submitInfo.commandBufferCount = 1;
                    submitInfo.pCommandBuffers = &font.GetVkCommandBuffer();
                }

                auto err = vkQueueSubmit(m_app->GetVkGraphicsQueue(), 1, &submitInfo, VK_NULL_HANDLE);
                assert(err == VkResult::VK_SUCCESS);
                err = vkQueueWaitIdle(m_app->GetVkGraphicsQueue());
                assert(err == VkResult::VK_SUCCESS);
                ImGui_ImplVulkan_DestroyFontUploadObjects();
            }

        }
        virtual void NewFrame() override
        {
            m_app->GetWindow()->EnumEvents([](void* event) {
               ImGui_ImplSDL2_ProcessEvent((SDL_Event*)event);
            });
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplSDL2_NewFrame();
            ImGui::NewFrame();
        }
        virtual void EndFrame() override
        {
            ImGui::EndFrame();
        }
        virtual void Terminate() override
        {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplSDL2_Shutdown();
            ImGui::DestroyContext();
        }
        virtual void Render(gfx::GFXCommandBuffer* cmd) override
        {
            ImGui::Render();
            ImDrawData* draw_data = ImGui::GetDrawData();
            const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
            if (!is_minimized)
            {
                auto vkCmd = dynamic_cast<gfx::GFXVulkanCommandBuffer*>(cmd);
                ImGui_ImplVulkan_RenderDrawData(draw_data, vkCmd->GetVkCommandBuffer());
            }
        }

        void SetLayoutInfo(jxcorlib::string_view content) override
        {
            ImGui::LoadIniSettingsFromMemory(content.data());
        }

        std::string GetLayoutInfo() override
        {
            size_t size;
            const char* str = ImGui::SaveIniSettingsToMemory(&size);
            return std::string{ std::string_view{str, size} };
        }

    };
#endif

    std::shared_ptr<ImGuiObject> CreateImGui(gfx::GFXApplication* app)
    {

        switch (app->GetApiType())
        {
#if PULSAR_GFX_BUILD_VULKAN
        case gfx::GFXApi::Vulkan:
            return std::shared_ptr<ImGuiObject>(new ImGuiImpl_Vulkan(static_cast<gfx::GFXVulkanApplication*>(app)));
            break;
#endif
        default:
            break;
        }

        return nullptr;
    }
}