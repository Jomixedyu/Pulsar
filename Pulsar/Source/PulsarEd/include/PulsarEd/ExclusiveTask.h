#pragma once
#include <PulsarEd/Assembly.h>
#include <queue>
#include <imgui/imgui.h>

namespace pulsared
{
    struct ExclusiveTaskProcessInfo
    {
        string Description;
        float  Percentage;
    };

    enum class ExclusiveTaskState
    {
        Continue,
        Success,
        Failure,
    };

    class ExclusiveTask : public jxcorlib::Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsared, pulsared::ExclusiveTask, jxcorlib::Object);
    public:
        virtual ExclusiveTaskState OnProcess(ExclusiveTaskProcessInfo& info)
        {
            info.Description = "processing";
            return ExclusiveTaskState::Continue;
        }
        virtual bool                CanCancel() const { return true; }
        virtual void                OnComplete() {}

        bool IsCancel;
    };
    CORELIB_DECL_SHORTSPTR(ExclusiveTask);

    class ExclusiveTaskQueue
    {
        std::queue<ExclusiveTask_sp> m_tasks;
    public:


        void AddTask(ExclusiveTask_rsp task)
        {
            m_tasks.push(task);
        }
        static inline const char* WindowName = "__ExclusiveTaskQueue";

        void TickDraw()
        {
            /*string title = std::to_string(m_tasks.size()) + " tasks";

            ImGui::PushID(WindowName);
            if (m_tasks.size() != 0)
            {
                ImGui::OpenPopup(title.c_str());
                ImGui::SetNextWindowSize({ 500,150 });
            }
            auto winFlag = ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize;

            if (ImGui::BeginPopupModal((std::to_string(m_tasks.size()) + " tasks").c_str(), nullptr, winFlag))
            {
                auto& task = m_tasks.front();
                ExclusiveTaskProcessInfo info;

                auto state = task->OnProcess(info);

                ImGui::Text(info.Description.c_str());
                ImGui::ProgressBar(info.Percentage);

                bool canCancel = task->CanCancel();
                if (canCancel)
                {
                    ImGui::BeginDisabled();
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel"))
                {
                    task->IsCancel = true;
                }
                if (canCancel)
                {
                    ImGui::EndDisabled();
                }

                switch (state)
                {
                case pulsared::ExclusiveTaskState::Continue:
                {
                }
                break;
                case pulsared::ExclusiveTaskState::Success:
                    m_tasks.pop();
                    break;
                case pulsared::ExclusiveTaskState::Failure:
                    m_tasks.pop();
                    break;
                default:
                    break;
                }

                ImGui::EndPopup();
            }

            ImGui::PopID();*/

        }
    };
}