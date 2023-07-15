#include "ImGuiImpl.h"
#include <Pulsar/ImGuiImpl.h>
#include "DroidSans.ttf.h"
#include <ThirdParty/imgui/imgui.h>
#include <ThirdParty/imgui/imgui_impl_glfw.h>


#include <Pulsar/Private/SystemInterface.h>
#include <CoreLib.Platform/Window.h>
#include <gfx/GFXApplication.h>

#if PULSAR_GFX_BUILD_VULKAN
#include <ThirdParty/imgui/imgui_impl_vulkan.h>
#include <gfx-vk/GFXVulkanApplication.h>
#include <gfx-vk/GFXVulkanDescriptorManager.h>
#include <gfx-vk/GFXVulkanCommandBuffer.h>
#endif

namespace pulsar
{

    void ImGui_Style_Initialize()
    {

        //ImGui::StyleColorsDark();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        io.ConfigFlags |= ImGuiWindowFlags_MenuBar;
        //io.ConfigFlags |= ImGuiWindowFlags_NoTitleBar;
        io.ConfigFlags &= ~ImGuiConfigFlags_ViewportsEnable;

        io.ConfigWindowsMoveFromTitleBarOnly = true;

        //io.FontGlobalScale = jxcorlib::platform::window::GetUIScaling();
        io.MouseDrawCursor = true;

        io.Fonts->AddFontFromMemoryTTF(FILE_DroidSans_ttf, sizeof(FILE_DroidSans_ttf), 14.4f);
        //io.Fonts->AddFontFromFileTTF("DroidSans.ttf", 14.4f);
        ImGui::StyleColorsDark();

        ImGuiStyle& style = ImGui::GetStyle();


            style.WindowRounding = 0.0f;
            style.Colors[ImGuiCol_WindowBg].w = 1.0f;


        //custom
        {
            style.WindowPadding = { 2.f, 2.f };
            style.WindowRounding = 8;
            style.FrameRounding = 6;
            style.FramePadding = { 12,3 };
            style.ItemSpacing = { 10,5 };
            style.ScrollbarSize = 10;

            //style.Colors[ImGuiCol_::ImGuiCol_Button] = ImColor(58, 58, 58, 255);

            //style.Colors[ImGuiCol_::ImGuiCol_WindowBg] = ImColor(26, 26, 26, 255);
            //style.Colors[ImGuiCol_::ImGuiCol_FrameBg] = ImColor(12, 12, 12, 255);
            //style.Colors[ImGuiCol_::ImGuiCol_TitleBg] = ImColor(25, 25, 28, 255);
            //style.Colors[ImGuiCol_::ImGuiCol_TitleBgActive] = ImColor(60, 60, 60, 255);

            ////tab
            //style.Colors[ImGuiCol_::ImGuiCol_Tab] = ImColor(38, 38, 38, 255);
            //style.Colors[ImGuiCol_::ImGuiCol_TabActive] = ImColor(255, 79, 79, 255);
            //style.Colors[ImGuiCol_::ImGuiCol_TabHovered] = ImColor(255, 79, 79, 255);
            //style.Colors[ImGuiCol_::ImGuiCol_TabUnfocused] = ImColor(38, 38, 38, 255);
            //style.Colors[ImGuiCol_::ImGuiCol_TabUnfocusedActive] = ImColor(79, 79, 79, 255);

            //style.Colors[ImGuiCol_::ImGuiCol_Header] = ImColor(48, 48, 48, 255);

            //bug:
            //style.WindowMenuButtonPosition = ImGuiDir_::ImGuiDir_Right;

        }
        //style.Colors[ImGuiCol_Text] = ImColor(214, 214, 214, 255);
        //style.Colors[ImGuiCol_TextDisabled] = ImColor(110, 110, 110, 255);
        //style.Colors[ImGuiCol_WindowBg] = ImColor(31, 31, 31, 255);
        ////style.Colors[ImGuiCol_WindowBg] = ImColor(1, 0, 0, 255);

        //style.Colors[ImGuiCol_ChildBg] = ImColor(30, 30, 30, 128);
        //style.Colors[ImGuiCol_Border] = ImColor(140, 136, 138, 79);
        //style.Colors[ImGuiCol_FrameBg] = ImColor(66, 66, 66, 255);
        //style.Colors[ImGuiCol_FrameBgHovered] = ImColor(97, 96, 148, 102);
        //style.Colors[ImGuiCol_FrameBgActive] = ImColor(53, 46, 68, 172);
        //style.Colors[ImGuiCol_TitleBg] = ImColor(50, 50, 50, 255);
        //style.Colors[ImGuiCol_TitleBgActive] = ImColor(90, 90, 90, 255);
        //style.Colors[ImGuiCol_MenuBarBg] = ImColor(36, 36, 36, 255);
        //style.Colors[ImGuiCol_CheckMark] = ImColor(128, 99, 244, 255);
        //style.Colors[ImGuiCol_SliderGrab] = ImColor(120, 90, 205, 255);
        //style.Colors[ImGuiCol_SliderGrabActive] = ImColor(118, 66, 255, 255);
        //style.Colors[ImGuiCol_Button] = ImColor(66, 66, 66, 255);
        //style.Colors[ImGuiCol_ButtonHovered] = ImColor(64, 53, 130, 255);
        //style.Colors[ImGuiCol_ButtonActive] = ImColor(145, 131, 238, 255);
        //style.Colors[ImGuiCol_Header] = ImColor(101, 74, 169, 255);
        //style.Colors[ImGuiCol_HeaderHovered] = ImColor(66, 150, 126, 204);
        //style.Colors[ImGuiCol_HeaderActive] = ImColor(66, 150, 156, 255);
        //style.Colors[ImGuiCol_Tab] = ImColor(46, 46, 46, 255);
        //style.Colors[ImGuiCol_TabHovered] = ImColor(113, 96, 232, 255);
        //style.Colors[ImGuiCol_TabActive] = ImColor(113, 96, 232, 255);
        //style.Colors[ImGuiCol_TextSelectedBg] = ImColor(141, 98, 254, 255);
    }




#if PULSAR_GFX_BUILD_VULKAN
    class ImGuiImpl_Vulkan : public ImGuiObject
    {
    public:
        ImGuiImpl_Vulkan(gfx::GFXVulkanApplication* app)
            : m_app(app)
        { }


        gfx::GFXVulkanApplication* m_app;

        // Í¨¹ý ImGuiObject ¼Ì³Ð
        virtual void Initialize() override
        {
            IMGUI_CHECKVERSION();
            ImGui::CreateContext();
            //ImGuiIO& io = ImGui::GetIO(); (void)io;
            //ImGui::StyleColorsClassic();
            //ImGui::StyleColorsDark();
            ImGui_Style_Initialize();
            ImGui_ImplGlfw_InitForVulkan(m_app->GetWindow(), true);
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
                    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
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
            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
        }
        virtual void EndFrame() override
        {
        }
        virtual void Terminate() override
        {
            ImGui_ImplVulkan_Shutdown();
            ImGui_ImplGlfw_Shutdown();
            ImGui::DestroyContext();
        }
        virtual void Render(gfx::GFXCommandBuffer* cmd) override
        {
            ImGui::Render();
            ImDrawData* draw_data = ImGui::GetDrawData();
            const bool is_minimized = (draw_data->DisplaySize.x <= 0.0f || draw_data->DisplaySize.y <= 0.0f);
            if (!is_minimized)
            {
                auto vkCmd = static_cast<gfx::GFXVulkanCommandBuffer*>(cmd);
                ImGui_ImplVulkan_RenderDrawData(draw_data, vkCmd->GetVkCommandBuffer());
            }
        }
    };
#endif

    std::shared_ptr<ImGuiObject> CreateImGui(gfx::GFXApplication* app)
    {

        switch (app->GetApiType())
        {
#if PULSAR_GFX_BUILD_VULKAN
        case gfx::GFXVulkanApplication::StaticGetApiType():
            return std::shared_ptr<ImGuiObject>(new ImGuiImpl_Vulkan(static_cast<gfx::GFXVulkanApplication*>(app)));
            break;
#endif
        default:
            break;
        }

        return nullptr;
    }
}