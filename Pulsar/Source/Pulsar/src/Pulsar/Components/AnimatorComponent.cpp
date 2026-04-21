#include "Components/AnimatorComponent.h"
#include "Components/SkinnedMeshRendererComponent.h"
#include <Pulsar/Node.h>
#include <Pulsar/Logger.h>
#include <cmath>
#include <functional>

namespace pulsar
{
    AnimatorComponent::AnimatorComponent() = default;

    // -----------------------------------------------------------------------
    // Begin / End
    // -----------------------------------------------------------------------
    void AnimatorComponent::BeginComponent()
    {
        base::BeginComponent();
        // 仅初始化运行时参数表，不自动播放（播放在 BeginPlay 里触发）
        if (m_controller)
            m_runtimeParams = m_controller->GetParams();
    }

    void AnimatorComponent::EndComponent()
    {
        base::EndComponent();
        m_isPlaying = false;
        m_currentStateName.clear();
        m_runtimeParams.clear();
        m_boneMatrices.clear();
    }

    void AnimatorComponent::BeginPlay()
    {
        base::BeginPlay();
        if (m_controller && m_playOnStart)
        {
            m_runtimeParams = m_controller->GetParams();
            EnterState(m_controller->GetDefaultState());
        }
    }

    void AnimatorComponent::EndPlay()
    {
        base::EndPlay();
        m_isPlaying = false;
        m_currentStateName.clear();
        m_currentTime = 0.f;
        m_runtimeParams.clear();
        m_boneMatrices.clear();
    }

    void AnimatorComponent::SetController(RCPtr<AnimatorController> ctrl)
    {
        m_controller = std::move(ctrl);
        // 编辑器中只重置参数，不自动播放
        if (m_controller)
            m_runtimeParams = m_controller->GetParams();
    }

    void AnimatorComponent::PostEditChange(FieldInfo* info)
    {
        base::PostEditChange(info);
        if (info->GetName() == NAMEOF(m_controller))
        {
            // Inspector 里拖入 Controller 后，仅重置状态机，不自动播放
            // PlayOnStart 只在运行时 BeginComponent 里生效
            m_runtimeParams.clear();
            m_currentStateName.clear();
            m_isPlaying = false;
            m_currentTime = 0.f;
            if (m_controller)
                m_runtimeParams = m_controller->GetParams();
        }
    }

    // -----------------------------------------------------------------------
    // Parameter access
    // -----------------------------------------------------------------------
    AnimatorParam* FindRuntimeParam(array_list<AnimatorParam>& params, const string& name)
    {
        for (auto& p : params) if (p.Name == name) return &p;
        return nullptr;
    }
    const AnimatorParam* FindRuntimeParamConst(const array_list<AnimatorParam>& params, const string& name)
    {
        for (auto& p : params) if (p.Name == name) return &p;
        return nullptr;
    }

    void AnimatorComponent::SetBool(const string& name, bool value)
    {
        if (auto* p = FindRuntimeParam(m_runtimeParams, name)) p->BoolValue = value;
    }
    void AnimatorComponent::SetInt(const string& name, int value)
    {
        if (auto* p = FindRuntimeParam(m_runtimeParams, name)) p->IntValue = value;
    }
    void AnimatorComponent::SetFloat(const string& name, float value)
    {
        if (auto* p = FindRuntimeParam(m_runtimeParams, name)) p->FloatValue = value;
    }
    void AnimatorComponent::SetTrigger(const string& name)
    {
        if (auto* p = FindRuntimeParam(m_runtimeParams, name))
            if (p->Type == AnimatorParamType::Trigger) p->BoolValue = true;
    }
    bool AnimatorComponent::GetBool(const string& name) const
    {
        if (auto* p = FindRuntimeParamConst(m_runtimeParams, name)) return p->BoolValue;
        return false;
    }
    int AnimatorComponent::GetInt(const string& name) const
    {
        if (auto* p = FindRuntimeParamConst(m_runtimeParams, name)) return p->IntValue;
        return 0;
    }
    float AnimatorComponent::GetFloat(const string& name) const
    {
        if (auto* p = FindRuntimeParamConst(m_runtimeParams, name)) return p->FloatValue;
        return 0.f;
    }

    float AnimatorComponent::GetNormalizedTime() const
    {
        if (!m_controller) return 0.f;
        const AnimatorState* state = m_controller->FindState(m_currentStateName);
        if (!state || !state->Clip || state->Clip->GetDuration() <= 0.f) return 0.f;
        return m_currentTime / state->Clip->GetDuration();
    }

    // -----------------------------------------------------------------------
    // State machine
    // -----------------------------------------------------------------------
    void AnimatorComponent::EnterState(const string& name)
    {
        m_currentStateName = name;
        m_currentTime      = 0.f;
        m_isPlaying        = !name.empty();
    }

    bool AnimatorComponent::EvaluateConditions(const AnimatorTransition& tr) const
    {
        for (auto& cond : tr.Conditions)
        {
            auto* p = FindRuntimeParamConst(m_runtimeParams, cond.ParamName);
            if (!p) return false;

            switch (p->Type)
            {
            case AnimatorParamType::Trigger:
                if (!p->BoolValue) return false;
                break;
            case AnimatorParamType::Bool:
                if (cond.Op == AnimatorConditionOp::Equals    && p->BoolValue != (cond.Threshold >= 0.5f)) return false;
                if (cond.Op == AnimatorConditionOp::NotEquals && p->BoolValue == (cond.Threshold >= 0.5f)) return false;
                break;
            case AnimatorParamType::Int:
            {
                int iv = p->IntValue, th = (int)cond.Threshold;
                if (cond.Op == AnimatorConditionOp::Equals    && iv != th) return false;
                if (cond.Op == AnimatorConditionOp::NotEquals && iv == th) return false;
                if (cond.Op == AnimatorConditionOp::Greater   && iv <= th) return false;
                if (cond.Op == AnimatorConditionOp::Less      && iv >= th) return false;
                break;
            }
            case AnimatorParamType::Float:
            {
                float fv = p->FloatValue, th = cond.Threshold;
                if (cond.Op == AnimatorConditionOp::Greater && fv <= th) return false;
                if (cond.Op == AnimatorConditionOp::Less    && fv >= th) return false;
                if (cond.Op == AnimatorConditionOp::Equals  && std::abs(fv - th) > 1e-5f) return false;
                break;
            }
            }
        }
        return true;
    }

    void AnimatorComponent::ConsumeTriggersOfTransition(const AnimatorTransition& tr)
    {
        for (auto& cond : tr.Conditions)
        {
            auto* p = FindRuntimeParam(m_runtimeParams, cond.ParamName);
            if (p && p->Type == AnimatorParamType::Trigger)
                p->BoolValue = false;
        }
    }

    // -----------------------------------------------------------------------
    // Tick
    // -----------------------------------------------------------------------
    void AnimatorComponent::OnTick(Ticker ticker)
    {
        base::OnTick(ticker);

        if (!m_isPlaying || !m_controller || m_currentStateName.empty())
            return;

        const AnimatorState* state = m_controller->FindState(m_currentStateName);
        if (!state || !state->Clip) return;

        const float duration = state->Clip->GetDuration();
        m_currentTime += ticker.deltatime * m_speed * state->Speed;

        // 检查过渡（AnyState + 当前 State 的过渡）
        for (auto& tr : m_controller->GetTransitions())
        {
            bool matchFrom = tr.FromState.empty() || tr.FromState == m_currentStateName;
            if (!matchFrom) continue;

            // 防止 Any->当前状态 / 当前状态->当前状态 的自循环过渡导致每帧 EnterState，时间永远归零
            if (tr.ToState == m_currentStateName)
                continue;

            // HasExitTime 条件
            if (tr.HasExitTime && duration > 0.f)
            {
                float normalized = m_currentTime / duration;
                if (normalized < tr.ExitTime) continue;
            }

            if (EvaluateConditions(tr))
            {
                ConsumeTriggersOfTransition(tr);
                EnterState(tr.ToState);
                // 重新取新状态
                state = m_controller->FindState(m_currentStateName);
                if (!state || !state->Clip) return;
                break;
            }
        }

        // 循环/停止
        const float newDuration = state->Clip->GetDuration();
        if (newDuration > 0.f && m_currentTime > newDuration)
        {
            if (state->Loop)
                m_currentTime = std::fmod(m_currentTime, newDuration);
            else
            {
                m_currentTime = newDuration;
                m_isPlaying = false;
            }
        }

        SampleAt(m_currentTime);
    }

    // -----------------------------------------------------------------------
    // Sampling helpers
    // -----------------------------------------------------------------------

    void AnimatorComponent::SampleAt(float time)
    {
        if (!m_controller) return;
        const AnimatorState* state = m_controller->FindState(m_currentStateName);
        if (!state || !state->Clip) return;

        auto skeleton = state->Clip->GetSkeleton();
        if (!skeleton)
        {
            Logger::Log("AnimatorComponent::SampleAt - state clip has no skeleton, state=" + m_currentStateName, LogLevel::Warning);
            return;
        }

        const auto& bones = skeleton->GetBones();
        // 以 Animator 所在节点作为导入小场景根；Skeleton::BoneInfo::Path 也相对这个根。
        auto* rootTransform = GetNode()->GetTransform().GetPtr();

        int missingTrackCount = 0;
        int missingNodeCount = 0;
        int appliedCount = 0;

        for (const auto& bone : bones)
        {
            auto* track = state->Clip->FindTrack(bone.Name);
            if (!track)
            {
                if (missingTrackCount < 5)
                    Logger::Log("AnimatorComponent::SampleAt - track not found for bone: " + bone.Name, LogLevel::Warning);
                ++missingTrackCount;
                continue;
            }

            // 优先按层级路径找；旧资源没有 Path 时 fallback 到直接子节点名字查找
            auto boneTransform = !bone.Path.empty()
                ? rootTransform->FindByPath(bone.Path)
                : rootTransform->FindByName(bone.Name);
            if (!boneTransform)
            {
                if (missingNodeCount < 5)
                    Logger::Log("AnimatorComponent::SampleAt - node not found for bone: " + bone.Name + ", path=" + bone.Path, LogLevel::Warning);
                ++missingNodeCount;
                continue;
            }

            // 采样局部 TRS
            Vector3f pos   = SampleVector3(track->PositionKeys, time);
            Quat4f   rot   = SampleQuat   (track->RotationKeys, time);
            Vector3f scale = SampleVector3(track->ScaleKeys,    time);

            boneTransform->SetPosition(pos);
            boneTransform->SetRotation(rot);
            boneTransform->SetScale(scale);
            ++appliedCount;


        }

        if (appliedCount == 0 || missingTrackCount > 0 || missingNodeCount > 0)
        {
            Logger::Log(
                "AnimatorComponent::SampleAt summary - state=" + m_currentStateName +
                ", time=" + std::to_string(time) +
                ", bones=" + std::to_string((int)bones.size()) +
                ", applied=" + std::to_string(appliedCount) +
                ", missingTrack=" + std::to_string(missingTrackCount) +
                ", missingNode=" + std::to_string(missingNodeCount),
                (appliedCount > 0 && missingTrackCount == 0 && missingNodeCount == 0) ? LogLevel::Info : LogLevel::Warning);
        }
    }

    void AnimatorComponent::EnterDefaultState()
    {
        if (!m_controller) return;
        const string& defaultState = m_controller->GetDefaultState();
        if (!defaultState.empty())
        {
            EnterState(defaultState);
        }
        else if (!m_controller->GetStates().empty())
        {
            EnterState(m_controller->GetStates().front().Name);
        }
    }

    void AnimatorComponent::StopPreview()
    {
        m_currentStateName.clear();
        m_currentTime = 0.f;
        m_isPlaying   = false;
        m_boneMatrices.clear();
    }

    void AnimatorComponent::GetDependenciesAsset(array_list<guid_t>& deps) const
    {
        base::GetDependenciesAsset(deps);
        if (m_controller) deps.push_back(m_controller.GetGuid());
    }

    // -----------------------------------------------------------------------
    // Keyframe sampling (linear interpolation)
    // -----------------------------------------------------------------------
    Vector3f AnimatorComponent::SampleVector3(const array_list<AnimVector3Key>& keys, float time)
    {
        if (keys.empty()) return {};
        if (keys.size() == 1 || time <= keys.front().Time) return keys.front().Value;
        if (time >= keys.back().Time) return keys.back().Value;

        int lo = 0, hi = (int)keys.size() - 1;
        while (lo + 1 < hi) { int m = (lo + hi) / 2; (keys[m].Time <= time ? lo : hi) = m; }

        float alpha = (keys[hi].Time > keys[lo].Time)
            ? (time - keys[lo].Time) / (keys[hi].Time - keys[lo].Time) : 0.f;
        const auto& a = keys[lo].Value; const auto& b = keys[hi].Value;
        return { a.x + (b.x-a.x)*alpha, a.y + (b.y-a.y)*alpha, a.z + (b.z-a.z)*alpha };
    }

    Quat4f AnimatorComponent::SampleQuat(const array_list<AnimQuatKey>& keys, float time)
    {
        if (keys.empty()) return {};
        if (keys.size() == 1 || time <= keys.front().Time) return keys.front().Value;
        if (time >= keys.back().Time) return keys.back().Value;

        int lo = 0, hi = (int)keys.size() - 1;
        while (lo + 1 < hi) { int m = (lo + hi) / 2; (keys[m].Time <= time ? lo : hi) = m; }

        float alpha = (keys[hi].Time > keys[lo].Time)
            ? (time - keys[lo].Time) / (keys[hi].Time - keys[lo].Time) : 0.f;

        const Quat4f& qa = keys[lo].Value;
        Quat4f qb = keys[hi].Value;
        float dot = Quat4f::Dot(qa, qb);
        if (dot < 0.f) { qb.x=-qb.x; qb.y=-qb.y; qb.z=-qb.z; qb.w=-qb.w; dot=-dot; }

        Quat4f result;
        if (dot > 0.9995f)
        {
            result = { qa.x+(qb.x-qa.x)*alpha, qa.y+(qb.y-qa.y)*alpha,
                       qa.z+(qb.z-qa.z)*alpha, qa.w+(qb.w-qa.w)*alpha };
        }
        else
        {
            float t0 = std::acos(dot), t = t0*alpha;
            float s0 = std::cos(t) - dot*std::sin(t)/std::sin(t0);
            float s1 = std::sin(t) / std::sin(t0);
            result = { s0*qa.x+s1*qb.x, s0*qa.y+s1*qb.y, s0*qa.z+s1*qb.z, s0*qa.w+s1*qb.w };
        }
        return Quat4f::NormalizeSafe(result);
    }

} // namespace pulsar
