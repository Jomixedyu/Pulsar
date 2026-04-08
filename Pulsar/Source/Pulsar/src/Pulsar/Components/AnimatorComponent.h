#pragma once
#include "Component.h"
#include "Assets/AnimationClip.h"
#include "Assets/Skeleton.h"

namespace pulsar
{
    class SkinnedMeshRendererComponent;

    class AnimatorComponent : public Component
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::AnimatorComponent, Component);
        CORELIB_CLASS_ATTR(new CategoryAttribute("Animation"));
    public:
        AnimatorComponent();

        void BeginComponent() override;
        void EndComponent() override;
        void OnTick(Ticker ticker) override;
        void GetDependenciesAsset(array_list<jxcorlib::guid_t>& deps) const override;

        // 播放控制
        void Play(RCPtr<AnimationClip> clip, bool loop = true);
        void Stop();
        void SetSpeed(float speed) { m_speed = speed; }
        float GetSpeed() const { return m_speed; }
        bool IsPlaying() const { return m_isPlaying; }
        float GetCurrentTime() const { return m_currentTime; }

        RCPtr<AnimationClip> GetCurrentClip() const { return m_currentClip; }

    protected:
        void SampleAndUpload(float time);

        // 线性插值工具
        static Vector3f SampleVector3(const array_list<AnimVector3Key>& keys, float time);
        static Quat4f   SampleQuat(const array_list<AnimQuatKey>& keys, float time);

    protected:
        CORELIB_REFL_DECL_FIELD(m_defaultClip);
        RCPtr<AnimationClip> m_defaultClip;   // 编辑器设置，BeginPlay 时自动播放

        CORELIB_REFL_DECL_FIELD(m_loop);
        bool m_loop = true;

        CORELIB_REFL_DECL_FIELD(m_playOnStart);
        bool m_playOnStart = true;

        CORELIB_REFL_DECL_FIELD(m_speed);
        float m_speed = 1.0f;

    private:
        RCPtr<AnimationClip>          m_currentClip;
        float                         m_currentTime = 0.f;
        bool                          m_isPlaying   = false;

        // 缓存同节点上的 SkinnedMeshRendererComponent
        ObjectPtr<SkinnedMeshRendererComponent> m_renderer;
    };

} // namespace pulsar
