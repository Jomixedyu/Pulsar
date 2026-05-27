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

        for (auto& trackBase : state->Clip->GetTracks())
        {
            switch (trackBase->TrackType)
            {
            case AnimationTrackType::Bone:
                if (auto boneTrack = sptr_cast<BoneAnimationTrack>(trackBase))
                    SampleBoneTrack(boneTrack.get(), time);
                break;
            case AnimationTrackType::Property:
                if (auto propTrack = sptr_cast<PropertyAnimationTrack>(trackBase))
                    SamplePropertyTrack(propTrack.get(), time);
                break;
            }
        }
    }

    void AnimatorComponent::SampleBoneTrack(BoneAnimationTrack* track, float time)
    {
        auto* rootTransform = GetNode()->GetTransform().GetPtr();
        if (!rootTransform) return;

        auto boneTransform = rootTransform->FindByName(track->BoneName);
        if (!boneTransform)
        {
            static hash_set<string> s_loggedBones;
            if (s_loggedBones.insert(track->BoneName).second)
                Logger::Log("AnimatorComponent::SampleBoneTrack - node not found for bone: " + track->BoneName, LogLevel::Warning);
            return;
        }

        Vector3f pos   = SampleVector3(track->PositionKeys, time);
        Quat4f   rot   = SampleQuat   (track->RotationKeys, time);
        Vector3f scale = SampleVector3(track->ScaleKeys,    time);

        boneTransform->SetPosition(pos);
        boneTransform->SetRotation(rot);
        boneTransform->SetScale(scale);
    }

    void AnimatorComponent::SamplePropertyTrack(PropertyAnimationTrack* track, float time)
    {
        auto node = GetNode();
        if (!node) return;

        // 解析 Component 类型名 -> Type*
        static hash_map<string, Type*> s_typeCache;
        Type* compType = nullptr;
        auto it = s_typeCache.find(track->ComponentTypeName);
        if (it != s_typeCache.end())
        {
            compType = it->second;
        }
        else
        {
            compType = AssemblyManager::GlobalFindType(track->ComponentTypeName);
            if (compType) s_typeCache[track->ComponentTypeName] = compType;
        }
        if (!compType)
        {
            static hash_set<string> s_loggedTypes;
            if (s_loggedTypes.insert(track->ComponentTypeName).second)
                Logger::Log("AnimatorComponent::SamplePropertyTrack - component type not found: " + track->ComponentTypeName, LogLevel::Warning);
            return;
        }

        // 查找目标 Component 实例
        Object* compInstance = nullptr;
        for (auto& comp : node->GetAllComponentArray())
        {
            if (comp->GetType() == compType || comp->GetType()->IsSubclassOf(compType))
            {
                compInstance = comp.GetPtr();
                break;
            }
        }
        if (!compInstance)
        {
            static hash_set<string> s_loggedComps;
            auto key = track->ComponentTypeName + "::" + track->FieldName;
            if (s_loggedComps.insert(key).second)
                Logger::Log("AnimatorComponent::SamplePropertyTrack - component not found on node: " + track->ComponentTypeName, LogLevel::Warning);
            return;
        }

        // 获取 FieldInfo
        auto* fieldInfo = compType->GetFieldInfo(track->FieldName);
        if (!fieldInfo)
        {
            static hash_set<string> s_loggedFields;
            auto key = track->ComponentTypeName + "::" + track->FieldName;
            if (s_loggedFields.insert(key).second)
                Logger::Log("AnimatorComponent::SamplePropertyTrack - field not found: " + key, LogLevel::Warning);
            return;
        }

        // 根据字段类型采样并写入
        Type* fieldType = fieldInfo->GetFieldType();
        if (fieldType == cltypeof<jxcorlib::Single32>())
        {
            if (track->FloatKeys.empty()) return;
            float value = SampleFloat(track->FloatKeys, time);
            fieldInfo->SetValue(compInstance, BoxUtil::Box(value));
        }
        else if (fieldType == cltypeof<jxcorlib::math::BoxingVector3f>())
        {
            if (track->Vector3Keys.empty()) return;
            Vector3f value = SampleVector3(track->Vector3Keys, time);
            fieldInfo->SetValue(compInstance, BoxUtil::Box(value));
        }
        else if (fieldType == cltypeof<jxcorlib::math::BoxingQuat4f>())
        {
            if (track->QuatKeys.empty()) return;
            Quat4f value = SampleQuat(track->QuatKeys, time);
            fieldInfo->SetValue(compInstance, BoxUtil::Box(value));
        }
        else
        {
            static hash_set<string> s_loggedTypes;
            auto key = track->ComponentTypeName + "::" + track->FieldName + " [type=" + fieldType->GetShortName() + "]";
            if (s_loggedTypes.insert(key).second)
                Logger::Log("AnimatorComponent::SamplePropertyTrack - unsupported field type: " + key, LogLevel::Warning);
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
    float AnimatorComponent::SampleFloat(const array_list<AnimFloatKey>& keys, float time)
    {
        if (keys.empty()) return 0.f;
        if (keys.size() == 1 || time <= keys.front().Time) return keys.front().Value;
        if (time >= keys.back().Time) return keys.back().Value;

        int lo = 0, hi = (int)keys.size() - 1;
        while (lo + 1 < hi) { int m = (lo + hi) / 2; (keys[m].Time <= time ? lo : hi) = m; }

        float alpha = (keys[hi].Time > keys[lo].Time)
            ? (time - keys[lo].Time) / (keys[hi].Time - keys[lo].Time) : 0.f;
        return keys[lo].Value + (keys[hi].Value - keys[lo].Value) * alpha;
    }

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
