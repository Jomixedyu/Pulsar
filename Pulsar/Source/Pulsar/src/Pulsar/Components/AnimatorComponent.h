#pragma once
#include "Component.h"
#include "../Assets/AnimatorController.h"

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
        void BeginPlay() override;
        void EndPlay() override;
        void OnTick(Ticker ticker) override;
        void PostEditChange(FieldInfo* info) override;
        void GetDependenciesAsset(array_list<jxcorlib::guid_t>& deps) const override;

        // --- Controller ---
        RCPtr<AnimatorController> GetController() const { return m_controller; }
        void SetController(RCPtr<AnimatorController> ctrl);

        // --- 参数设置（运行时调用，驱动状态切换）---
        void SetBool   (const string& name, bool  value);
        void SetInt    (const string& name, int   value);
        void SetFloat  (const string& name, float value);
        void SetTrigger(const string& name);

        bool  GetBool (const string& name) const;
        int   GetInt  (const string& name) const;
        float GetFloat(const string& name) const;

        // --- 状态查询 ---
        const string&              GetCurrentStateName() const { return m_currentStateName; }
        float                      GetNormalizedTime()   const;
        bool                       IsPlaying()           const { return m_isPlaying; }
        // 供 SkinnedMeshRendererComponent 每帧拉取
        const array_list<Matrix4f>& GetBoneMatrices()    const { return m_boneMatrices; }

        // --- 编辑器预览用：手动采样（直接传 renderer）---
        void SampleAt(float time);

        // --- 编辑器预览快捷接口 ---
        void EnterDefaultState();  // 进入 Controller 的默认状态并开始播放
        void StopPreview();        // 停止预览，清空当前状态

    protected:
        static Vector3f SampleVector3(const array_list<AnimVector3Key>& keys, float time);
        static Quat4f   SampleQuat   (const array_list<AnimQuatKey>& keys,    float time);

    private:
        void EnterState(const string& name);
        bool EvaluateConditions(const AnimatorTransition& transition) const;
        void ConsumeTriggersOfTransition(const AnimatorTransition& transition);

    protected:
        CORELIB_REFL_DECL_FIELD(m_controller);
        RCPtr<AnimatorController> m_controller;

        CORELIB_REFL_DECL_FIELD(m_playOnStart);
        bool m_playOnStart = true;

        CORELIB_REFL_DECL_FIELD(m_speed);
        float m_speed = 1.0f;

    private:
        array_list<AnimatorParam> m_runtimeParams;
        array_list<Matrix4f>      m_boneMatrices;

        CORELIB_REFL_DECL_FIELD(m_currentStateName, new DebugPropertyAttribute(), new ReadOnlyPropertyAttribute(), new NoSerializableAttribtue());
        string  m_currentStateName;

        CORELIB_REFL_DECL_FIELD(m_currentTime, new DebugPropertyAttribute(), new ReadOnlyPropertyAttribute(), new NoSerializableAttribtue());
        float   m_currentTime = 0.f;

        CORELIB_REFL_DECL_FIELD(m_isPlaying, new DebugPropertyAttribute(), new ReadOnlyPropertyAttribute(), new NoSerializableAttribtue());
        bool    m_isPlaying   = false;
    };

} // namespace pulsar
