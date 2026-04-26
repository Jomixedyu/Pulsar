#include "Assets/AnimatorController.h"
#include "AssetSerializerUtil.h"
#include <Pulsar/AssetObject.h>

namespace pulsar
{
    // -----------------------------------------------------------------------
    // 序列化辅助
    // -----------------------------------------------------------------------
    static void SerializeParam(std::iostream& s, bool w, AnimatorParam& p)
    {
        sser::ReadWriteStream(s, w, p.Name);
        sser::ReadWriteStream(s, w, (int32_t&)p.Type);
        sser::ReadWriteStream(s, w, p.FloatValue);
        sser::ReadWriteStream(s, w, p.IntValue);
        sser::ReadWriteStream(s, w, p.BoolValue);
    }

    static void SerializeCondition(std::iostream& s, bool w, AnimatorCondition& c)
    {
        sser::ReadWriteStream(s, w, c.ParamName);
        sser::ReadWriteStream(s, w, (int32_t&)c.Op);
        sser::ReadWriteStream(s, w, c.Threshold);
    }

    static void SerializeTransition(std::iostream& s, bool w, AnimatorTransition& t)
    {
        sser::ReadWriteStream(s, w, t.FromState);
        sser::ReadWriteStream(s, w, t.ToState);
        sser::ReadWriteStream(s, w, t.HasExitTime);
        sser::ReadWriteStream(s, w, t.ExitTime);
        sser::ReadWriteStream(s, w, t.Duration);

        uint32_t count = (uint32_t)t.Conditions.size();
        sser::ReadWriteStream(s, w, count);
        if (!w) t.Conditions.resize(count);
        for (auto& c : t.Conditions)
            SerializeCondition(s, w, c);
    }

    // -----------------------------------------------------------------------
    // AnimatorController
    // -----------------------------------------------------------------------
    RCPtr<AnimatorController> AnimatorController::StaticCreate(string_view name)
    {
        auto self = NewAssetObject<AnimatorController>();
        self->SetIndexName(name);
        return self;
    }

    // --- States ---
    void AnimatorController::AddState(AnimatorState state)
    {
        m_states.push_back(std::move(state));
        if (m_defaultState.empty() && !m_states.empty())
            m_defaultState = m_states.front().Name;
    }
    void AnimatorController::RemoveState(const string& name)
    {
        auto it = std::find_if(m_states.begin(), m_states.end(),
            [&](const AnimatorState& s) { return s.Name == name; });
        if (it != m_states.end()) m_states.erase(it);
        if (m_defaultState == name && !m_states.empty())
            m_defaultState = m_states.front().Name;
    }
    const AnimatorState* AnimatorController::FindState(const string& name) const
    {
        for (auto& s : m_states) if (s.Name == name) return &s;
        return nullptr;
    }
    AnimatorState* AnimatorController::FindState(const string& name)
    {
        for (auto& s : m_states) if (s.Name == name) return &s;
        return nullptr;
    }

    // --- Transitions ---
    void AnimatorController::AddTransition(AnimatorTransition transition)
    {
        m_transitions.push_back(std::move(transition));
    }
    void AnimatorController::RemoveTransition(int index)
    {
        if (index >= 0 && index < (int)m_transitions.size())
            m_transitions.erase(m_transitions.begin() + index);
    }

    // --- Parameters ---
    void AnimatorController::AddParam(AnimatorParam param)
    {
        m_params.push_back(std::move(param));
    }
    void AnimatorController::RemoveParam(const string& name)
    {
        auto it = std::find_if(m_params.begin(), m_params.end(),
            [&](const AnimatorParam& p) { return p.Name == name; });
        if (it != m_params.end()) m_params.erase(it);
    }
    const AnimatorParam* AnimatorController::FindParam(const string& name) const
    {
        for (auto& p : m_params) if (p.Name == name) return &p;
        return nullptr;
    }
    AnimatorParam* AnimatorController::FindParam(const string& name)
    {
        for (auto& p : m_params) if (p.Name == name) return &p;
        return nullptr;
    }

    // --- Serialize ---
    void AnimatorController::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        auto& stream  = s->GetStream();
        bool  isWrite = s->IsWrite;

        // DefaultState（JSON）
        if (isWrite)
            s->Object->Add("DefaultState", s->Object->New(ser::VarientType::String)->Assign(m_defaultState));
        else
            if (auto v = s->Object->At("DefaultState")) m_defaultState = v->AsString();

        // Params（binary stream）
        {
            uint32_t count = (uint32_t)m_params.size();
            sser::ReadWriteStream(stream, isWrite, count);
            if (!isWrite) m_params.resize(count);
            for (auto& p : m_params) SerializeParam(stream, isWrite, p);
        }

        // Transitions（binary stream）
        {
            uint32_t count = (uint32_t)m_transitions.size();
            sser::ReadWriteStream(stream, isWrite, count);
            if (!isWrite) m_transitions.resize(count);
            for (auto& t : m_transitions) SerializeTransition(stream, isWrite, t);
        }

        // States：name/speed/loop 走 binary stream，clip 走 GUID（JSON array）
        {
            uint32_t stateCount = (uint32_t)m_states.size();
            sser::ReadWriteStream(stream, isWrite, stateCount);
            if (!isWrite) m_states.resize(stateCount);
            for (auto& st : m_states)
            {
                sser::ReadWriteStream(stream, isWrite, st.Name);
                sser::ReadWriteStream(stream, isWrite, st.Speed);
                sser::ReadWriteStream(stream, isWrite, st.Loop);
            }

            // 兜底：States 读完后，如果 m_defaultState 为空则用第一个
            if (!isWrite && m_defaultState.empty() && !m_states.empty())
                m_defaultState = m_states.front().Name;
        }

        // State Clip GUIDs（JSON array）
        if (isWrite)
        {
            auto arr = s->Object->New(ser::VarientType::Array);
            for (auto& st : m_states)
            {
                auto guidStr = s->Object->New(ser::VarientType::String);
                guidStr->Assign(st.Clip ? st.Clip->GetAssetGuid().to_string() : "");
                arr->Push(guidStr);
            }
            s->Object->Add("StateClips", arr);
        }
        else
        {
            if (auto arr = s->Object->At("StateClips"))
            {
                for (int i = 0; i < (int)m_states.size() && i < (int)arr->GetCount(); ++i)
                {
                    auto guidStr = arr->At(i)->AsString();
                    if (!guidStr.empty())
                    {
                        auto guid = guid_t::parse(guidStr);
                        m_states[i].Clip = RuntimeAssetManager::GetLoadedAssetByGuid<AnimationClip>(guid);
                    }
                }
            }
        }
    }

    void AnimatorController::OnInstantiateAsset(AssetObject* obj)
    {
        auto* other = static_cast<AnimatorController*>(obj);
        other->m_defaultState = m_defaultState;
        other->m_params       = m_params;
        other->m_transitions  = m_transitions;
        // States：浅拷贝（Clip 是 RCPtr，共享引用）
        other->m_states.resize(m_states.size());
        for (size_t i = 0; i < m_states.size(); ++i)
        {
            other->m_states[i].Name  = m_states[i].Name;
            other->m_states[i].Clip  = m_states[i].Clip;
            other->m_states[i].Speed = m_states[i].Speed;
            other->m_states[i].Loop  = m_states[i].Loop;
        }
    }

    void AnimatorController::OnCollectAssetDependencies(array_list<jxcorlib::guid_t>& deps)
    {
        for (auto& st : m_states)
            if (st.Clip) deps.push_back(st.Clip->GetAssetGuid());
    }

} // namespace pulsar
