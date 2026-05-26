#include "Components/InputComponent.h"
#include "World.h"

namespace pulsar
{
    void InputComponent::BeginPlay()
    {
        base::BeginPlay();

        // Default bindings (hard-coded for now)
        BindAxis("Horizontal", KeyCode::A, KeyCode::D);
        BindAxis("Vertical",   KeyCode::S, KeyCode::W);
    }

    void InputComponent::OnTick(Ticker ticker)
    {
        base::OnTick(ticker);

        auto* ctx = GetWorld()->GetInputContext();
        if (!ctx)
            return;

        // Update action states
        for (auto& [name, state] : m_actions)
        {
            state.previous = state.current;
            state.current = ctx->GetKey(state.key);
        }

        // Update axis values
        for (auto& [name, axis] : m_axes)
        {
            float value = 0.0f;
            if (ctx->GetKey(axis.negativeKey))
                value -= 1.0f;
            if (ctx->GetKey(axis.positiveKey))
                value += 1.0f;
            axis.value = value * axis.scale;
        }
    }

    void InputComponent::BindAction(string_view name, KeyCode key)
    {
        m_actions[index_string(name)] = ActionState{ key, false, false };
    }

    void InputComponent::BindAxis(string_view name, KeyCode negativeKey, KeyCode positiveKey, float scale)
    {
        m_axes[index_string(name)] = AxisState{ negativeKey, positiveKey, scale, 0.0f };
    }

    bool InputComponent::GetAction(string_view name) const
    {
        auto it = m_actions.find(index_string(name));
        return it != m_actions.end() && it->second.current;
    }

    bool InputComponent::GetActionDown(string_view name) const
    {
        auto it = m_actions.find(index_string(name));
        return it != m_actions.end() && it->second.current && !it->second.previous;
    }

    bool InputComponent::GetActionUp(string_view name) const
    {
        auto it = m_actions.find(index_string(name));
        return it != m_actions.end() && !it->second.current && it->second.previous;
    }

    float InputComponent::GetAxis(string_view name) const
    {
        auto it = m_axes.find(index_string(name));
        return it != m_axes.end() ? it->second.value : 0.0f;
    }
}
