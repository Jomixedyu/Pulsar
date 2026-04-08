#include "Assets/AnimationClip.h"
#include "AssetSerializerUtil.h"
#include "EngineMath.h"
#include <Pulsar/AssetObject.h>

namespace pulsar
{
    // -----------------------------------------------------------------------
    // Serialization helpers
    // -----------------------------------------------------------------------
    static void SerializeAnimVector3Key(std::iostream& s, bool w, AnimVector3Key& k)
    {
        sser::ReadWriteStream(s, w, k.Time);
        ReadWriteStream(s, w, k.Value);
    }
    static void SerializeAnimQuatKey(std::iostream& s, bool w, AnimQuatKey& k)
    {
        sser::ReadWriteStream(s, w, k.Time);
        ReadWriteStream(s, w, k.Value);
    }
    static void SerializeBoneAnimTrack(std::iostream& s, bool w, BoneAnimTrack& t)
    {
        sser::ReadWriteStream(s, w, t.BoneName);

        auto writeKeys = [&](auto& keys, auto serFn)
        {
            uint32_t count = static_cast<uint32_t>(keys.size());
            sser::ReadWriteStream(s, w, count);
            if (!w) keys.resize(count);
            for (auto& k : keys) serFn(s, w, k);
        };
        writeKeys(t.PositionKeys, SerializeAnimVector3Key);
        writeKeys(t.RotationKeys, SerializeAnimQuatKey);
        writeKeys(t.ScaleKeys,    SerializeAnimVector3Key);
    }

    // -----------------------------------------------------------------------
    // AnimationClip
    // -----------------------------------------------------------------------
    RCPtr<AnimationClip> AnimationClip::StaticCreate(
        string_view                name,
        RCPtr<Skeleton>            skeleton,
        float                      duration,
        float                      fps,
        array_list<BoneAnimTrack>&& tracks)
    {
        auto self = NewAssetObject<AnimationClip>();
        self->SetIndexName(name);
        self->m_skeleton = skeleton;
        self->m_duration = duration;
        self->m_fps      = fps;
        self->m_tracks   = std::move(tracks);
        return self;
    }

    const BoneAnimTrack* AnimationClip::FindTrack(const string& boneName) const
    {
        for (auto& t : m_tracks)
            if (t.BoneName == boneName) return &t;
        return nullptr;
    }

    void AnimationClip::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        auto& stream  = s->GetStream();
        bool  isWrite = s->IsWrite;

        // --- Skeleton 引用（GUID）---
        if (isWrite)
        {
            auto guidStr = s->Object->New(ser::VarientType::String);
            guidStr->Assign(m_skeleton ? m_skeleton->GetAssetGuid().to_string() : "");
            s->Object->Add("Skeleton", guidStr);
        }
        else
        {
            if (auto obj = s->Object->At("Skeleton"))
            {
                auto guid = guid_t::parse(obj->AsString());
                m_skeleton = RuntimeAssetManager::GetLoadedAssetByGuid<Skeleton>(guid);
            }
        }

        // --- 基础属性（JSON）---
        if (isWrite)
        {
            s->Object->Add("Duration", s->Object->New(ser::VarientType::Number)->Assign(m_duration));
            s->Object->Add("FPS",      s->Object->New(ser::VarientType::Number)->Assign(m_fps));
        }
        else
        {
            if (auto v = s->Object->At("Duration")) m_duration = v->AsFloat();
            if (auto v = s->Object->At("FPS"))      m_fps      = v->AsFloat();
            m_tracks.clear();
        }

        // --- Tracks（二进制流）---
        uint32_t trackCount = static_cast<uint32_t>(m_tracks.size());
        sser::ReadWriteStream(stream, isWrite, trackCount);
        if (!isWrite) m_tracks.resize(trackCount);
        for (auto& t : m_tracks)
            SerializeBoneAnimTrack(stream, isWrite, t);
    }

    void AnimationClip::OnInstantiateAsset(AssetObject* obj)
    {
        base::OnInstantiateAsset(obj);
        auto clip      = static_cast<ThisClass*>(obj);
        clip->m_skeleton = m_skeleton;
        clip->m_duration = m_duration;
        clip->m_fps      = m_fps;
        clip->m_tracks   = m_tracks;
    }

} // namespace pulsar
