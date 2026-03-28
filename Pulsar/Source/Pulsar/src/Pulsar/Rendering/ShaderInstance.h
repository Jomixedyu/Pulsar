#pragma once
#include <Pulsar/Rendering/ShaderProgramResource.h>

#include <memory>
#include <mutex>

namespace pulsar
{
    enum class ShaderCompileState
    {
        Pending,
        Compiling,
        Ready,
        Error,
    };

    class ShaderInstance
    {
    public:
        explicit ShaderInstance(std::shared_ptr<ShaderProgramResource> fallback)
            : m_state(ShaderCompileState::Pending)
            , m_current(std::move(fallback))
        {
        }

        ~ShaderInstance() = default;

        ShaderCompileState GetState() const
        {
            std::lock_guard lock(m_mutex);
            return m_state;
        }

        std::shared_ptr<ShaderProgramResource> GetCurrentProgram() const
        {
            std::lock_guard lock(m_mutex);
            return m_current;
        }

        void ReplaceProgram(std::shared_ptr<ShaderProgramResource> program, ShaderCompileState newState)
        {
            std::lock_guard lock(m_mutex);
            m_current = std::move(program);
            m_state = newState;
        }

    private:
        mutable std::mutex m_mutex;
        ShaderCompileState m_state;
        std::shared_ptr<ShaderProgramResource> m_current;
    };
}
