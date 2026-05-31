#include "Gameplay/Character2dController.h"
#include "Gameplay/Character2d.h"
#include "Components/InputComponent.h"
#include "Logger.h"
#include "Node.h"
#include "World.h"

namespace pulsar
{
    Character2dController::Character2dController()
    {
        init_sptr_member(m_idleClip);
        init_sptr_member(m_moveClip);
        init_sptr_member(m_jumpUpClip);
        init_sptr_member(m_jumpHangClip);
        init_sptr_member(m_jumpFallClip);
        init_sptr_member(m_attackClip);
    }

    void Character2dController::BeginPlay()
    {
        base::BeginPlay();
        m_state = Character2dState::Idle;
        m_currentFrame = 0;
        m_frameTimer = 0.0f;
        m_jumpPhaseTimer = 0.0f;
        m_attackRequested = false;
        ApplyFrameToMaterial();
    }

    void Character2dController::EndPlay()
    {
        base::EndPlay();
    }

    void Character2dController::SetState(Character2dState state)
    {
        if (m_state != state)
        {
            m_state = state;
            m_currentFrame = 0;
            m_frameTimer = 0.0f;
            if (state == Character2dState::JumpUp)
                m_jumpPhaseTimer = 0.0f;
            ApplyFrameToMaterial();
        }
    }

    void Character2dController::Attack()
    {
        m_attackRequested = true;
    }

    SpriteAnimationClip* Character2dController::GetCurrentClip() const
    {
        switch (m_state)
        {
            case Character2dState::Idle:     return m_idleClip.get();
            case Character2dState::Move:     return m_moveClip.get();
            case Character2dState::JumpUp:   return m_jumpUpClip.get();
            case Character2dState::JumpHang: return m_jumpHangClip.get();
            case Character2dState::JumpFall: return m_jumpFallClip.get();
            case Character2dState::Attack:   return m_attackClip.get();
        }
        return nullptr;
    }

    void Character2dController::UpdateStateMachine(float dt)
    {
        auto* character = GetNode()->GetComponent<Character2d>().GetPtr();
        if (!character) return;

        auto input = GetNode()->GetComponent<InputComponent>().GetPtr();

        // Attack input has highest priority
        if (m_attackRequested)
        {
            m_attackRequested = false;
            if (m_state != Character2dState::Attack)
            {
                SetState(Character2dState::Attack);
                return;
            }
        }

        // If in attack, let it play out (user/code must switch back)
        if (m_state == Character2dState::Attack)
            return;

        bool jumpInput = input ? input->GetActionDown("Jump") : false;
        float h = input ? input->GetAxis("Horizontal") : 0.0f;

        // Jump trigger
        if (jumpInput && character->IsGrounded())
        {
            character->Jump();
            SetState(Character2dState::JumpUp);
            return;
        }

        // Jump phase transitions
        if (m_state == Character2dState::JumpUp)
        {
            m_jumpPhaseTimer += dt;
            if (m_jumpPhaseTimer >= m_jumpUpDuration)
            {
                SetState(Character2dState::JumpHang);
                m_jumpPhaseTimer = 0.0f;
            }
            return;
        }
        else if (m_state == Character2dState::JumpHang)
        {
            m_jumpPhaseTimer += dt;
            if (m_jumpPhaseTimer >= m_jumpHangDuration)
            {
                SetState(Character2dState::JumpFall);
                m_jumpPhaseTimer = 0.0f;
            }
            return;
        }
        else if (m_state == Character2dState::JumpFall)
        {
            if (character->IsGrounded())
            {
                if (std::abs(h) > 0.001f)
                    SetState(Character2dState::Move);
                else
                    SetState(Character2dState::Idle);
            }
            return;
        }

        // Grounded states
        if (std::abs(h) > 0.001f)
        {
            character->Move(h);
            if (m_state != Character2dState::Move)
                SetState(Character2dState::Move);
        }
        else
        {
            character->Move(0.0f);
            if (m_state != Character2dState::Idle)
                SetState(Character2dState::Idle);
        }
    }

    void Character2dController::UpdateAnimation(Ticker ticker)
    {
        auto* clip = GetCurrentClip();
        if (!clip || clip->frameCount <= 0)
            return;

        m_frameTimer += ticker.deltatime;
        float frameDuration = 1.0f / clip->framesPerSecond;

        while (m_frameTimer >= frameDuration)
        {
            m_frameTimer -= frameDuration;
            m_currentFrame++;

            if (m_state == Character2dState::Attack && m_currentFrame >= clip->frameCount)
            {
                auto input = GetNode()->GetComponent<InputComponent>().GetPtr();
                float h = input ? input->GetAxis("Horizontal") : 0.0f;
                if (std::abs(h) > 0.001f)
                    SetState(Character2dState::Move);
                else
                    SetState(Character2dState::Idle);
                return;
            }

            m_currentFrame %= clip->frameCount;
            ApplyFrameToMaterial();
        }
    }

    void Character2dController::ApplyFrameToMaterial()
    {
        if (!m_material)
            return;

        auto* clip = GetCurrentClip();
        if (!clip)
            return;

        int frameIndex = clip->startIndex + m_currentFrame;
        m_material->SetFloat("_Index", static_cast<float>(frameIndex));
        m_material->SubmitParameters();
    }

    void Character2dController::OnTick(Ticker ticker)
    {
        base::OnTick(ticker);

        UpdateStateMachine(ticker.deltatime);
        UpdateAnimation(ticker);
    }
}
