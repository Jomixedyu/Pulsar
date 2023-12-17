#pragma once
#include "Assembly.h"
#include "ExclusiveTask.h"
#include <Pulsar/AppInstance.h>
#include <Pulsar/AssetManager.h>
#include <Pulsar/EngineAppInstance.h>
#include <Pulsar/ImGuiImpl.h>

namespace pulsared
{
    struct ModalDialog
    {
        std::shared_ptr<ModalDialog> m_childModal;
        bool m_shouldClose = false;
        string m_name;
        explicit ModalDialog(const char* name)
        {
            m_name = name;
        }
        void Tick(float dt)
        {
            ImGui::OpenPopup(m_name.c_str());
            if (ImGui::BeginPopupModal(m_name.c_str(), nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                OnDraw(dt);
                if (m_childModal)
                {
                    m_childModal->Tick(dt);
                }
                if(m_shouldClose)
                {
                    ImGui::CloseCurrentPopup();
                }
                ImGui::EndPopup();
            }
        }
        virtual void OnDraw(float dt) = 0;
        virtual ~ModalDialog() = default;
    };

    class EditorAppInstance : public AppInstance
    {
    public:
        virtual const char* AppType() override;
        virtual void RequestQuit() override;
        virtual Vector2f GetOutputScreenSize() override;
        virtual void SetOutputScreenSize(Vector2f size) override;
        virtual string GetTitle() override;
        virtual void SetTitle(string_view title) override;
        virtual string AppRootDir() override;
        virtual void OnPreInitialize(gfx::GFXGlobalConfig* config) override;
        virtual void OnInitialized() override;
        virtual void OnTerminate() override;
        virtual void OnBeginRender(float dt) override;
        virtual void OnEndRender(float dt) override;
        virtual bool IsQuit() override;
        virtual rendering::Pipeline* GetPipeline() override;
        World* GetEditorWorld() const
        {
            return m_world;
        }
        virtual AssetManager* GetAssetManager() override
        {
            return m_assetManager;
        }

        virtual Vector2f GetAppSize();
        virtual void SetAppSize(Vector2f size);

        virtual bool IsInteractiveRendering() const;

        void StartInteractiveRendering();
        void StopInteractiveRendering();

        ExclusiveTaskQueue& GetTaskQueue()
        {
            return m_exclusiveTaskQueue;
        }

        void ShowModalDialog(sptr<ModalDialog> dialog);

    protected:
        sptr<ModalDialog> m_modalDialog;
        std::shared_ptr<ImGuiObject> m_gui = nullptr;

        AssetManager* m_assetManager{};
        World* m_world{};
        Vector2f m_outputSize{};
        ExclusiveTaskQueue m_exclusiveTaskQueue;

        bool m_isPlaying = false;
    };

    inline EditorAppInstance* GetEdApp()
    {
        return static_cast<EditorAppInstance*>(Application::inst());
    }
} // namespace pulsared