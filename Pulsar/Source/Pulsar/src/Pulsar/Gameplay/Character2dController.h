#pragma once
#include "Components/Component.h"
#include "Assets/Material.h"

namespace pulsar
{
    class Character2d;

    CORELIB_DEF_ENUM(AssemblyObject_pulsar, pulsar, Character2dState, Idle, Move, JumpUp, JumpHang, JumpFall, Attack);

    class SpriteAnimationClip : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::SpriteAnimationClip, Object);
    public:
        SpriteAnimationClip() = default;

        CORELIB_REFL_DECL_FIELD(startIndex);
        int startIndex = 0;

        CORELIB_REFL_DECL_FIELD(frameCount);
        int frameCount = 1;

        CORELIB_REFL_DECL_FIELD(framesPerSecond);
        float framesPerSecond = 10.0f;
    };

    class Character2dController : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::Character2dController, Component);
        CORELIB_CLASS_ATTR(new CategoryAttribute("2D"))
    public:
        Character2dController();

        void BeginPlay() override;
        void EndPlay() override;
        void OnTick(Ticker ticker) override;

        void SetState(Character2dState state);
        Character2dState GetState() const { return m_state; }

        void Attack();

    protected:
        void UpdateStateMachine(float dt);
        void UpdateAnimation(Ticker ticker);
        void ApplyFrameToMaterial();
        SpriteAnimationClip* GetCurrentClip() const;

        CORELIB_REFL_DECL_FIELD(m_material);
        RCPtr<Material> m_material;

        CORELIB_REFL_DECL_FIELD(m_idleClip);
        SPtr<SpriteAnimationClip> m_idleClip;

        CORELIB_REFL_DECL_FIELD(m_moveClip);
        SPtr<SpriteAnimationClip> m_moveClip;

        CORELIB_REFL_DECL_FIELD(m_jumpUpClip);
        SPtr<SpriteAnimationClip> m_jumpUpClip;

        CORELIB_REFL_DECL_FIELD(m_jumpHangClip);
        SPtr<SpriteAnimationClip> m_jumpHangClip;

        CORELIB_REFL_DECL_FIELD(m_jumpFallClip);
        SPtr<SpriteAnimationClip> m_jumpFallClip;

        CORELIB_REFL_DECL_FIELD(m_attackClip);
        SPtr<SpriteAnimationClip> m_attackClip;

        CORELIB_REFL_DECL_FIELD(m_jumpUpDuration);
        float m_jumpUpDuration = 0.2f;

        CORELIB_REFL_DECL_FIELD(m_jumpHangDuration);
        float m_jumpHangDuration = 0.2f;

        // runtime
        Character2dState m_state = Character2dState::Idle;
        int m_currentFrame = 0;
        float m_frameTimer = 0.0f;
        float m_jumpPhaseTimer = 0.0f;
        bool m_attackRequested = false;
    };
}

CORELIB_DECL_BOXING(pulsar::Character2dState, pulsar::BoxingCharacter2dState);
