#pragma once
#include <Pulsar/Rendering/ShaderProgram.h>

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

    class ShaderProgramInstance
    {
    public:
        explicit ShaderProgramInstance(std::shared_ptr<ShaderProgram> fallback)
            : m_state(ShaderCompileState::Pending)
            , m_current(std::move(fallback))
        {
        }

        ~ShaderProgramInstance() = default;

        ShaderCompileState GetState() const
        {
            std::lock_guard lock(m_mutex);
            return m_state;
        }

        std::shared_ptr<ShaderProgram> GetCurrentProgram() const
        {
            std::lock_guard lock(m_mutex);
            return m_current;
        }

        void ReplaceProgram(std::shared_ptr<ShaderProgram> program, ShaderCompileState newState)
        {
            std::lock_guard lock(m_mutex);
            m_current = std::move(program);
            m_state = newState;
        }

    private:
        mutable std::mutex m_mutex;
        ShaderCompileState m_state;
        std::shared_ptr<ShaderProgram> m_current;
    };
}
