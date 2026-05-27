#include "Assets/AnimationClip.h"
#include "AssetSerializerUtil.h"
#include "EngineMath.h"
#include <Pulsar/AssetObject.h>

namespace pulsar
{
    // -----------------------------------------------------------------------
    // Serialization helpers
    // -----------------------------------------------------------------------
    static void SerializeAnimFloatKey(std::iostream& s, bool w, AnimFloatKey& k)
    {
        sser::ReadWriteStream(s, w, k.Time);
        sser::ReadWriteStream(s, w, k.Value);
    }
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

    template<typename T>
    static void SerializeKeys(std::iostream& s, bool w, array_list<T>& keys, auto&& serFn)
    {
        uint32_t count = static_cast<uint32_t>(keys.size());
        sser::ReadWriteStream(s, w, count);
        if (!w) keys.resize(count);
        for (auto& k : keys) serFn(s, w, k);
    }

    static void SerializeBoneAnimTrack(std::iostream& s, bool w, BoneAnimationTrack& t)
    {
        sser::ReadWriteStream(s, w, t.BoneName);
        SerializeKeys(s, w, t.PositionKeys, SerializeAnimVector3Key);
        SerializeKeys(s, w, t.RotationKeys, SerializeAnimQuatKey);
        SerializeKeys(s, w, t.ScaleKeys,    SerializeAnimVector3Key);
    }

    static void SerializePropertyAnimTrack(std::iostream& s, bool w, PropertyAnimationTrack& t)
    {
        sser::ReadWriteStream(s, w, t.ComponentTypeName);
        sser::ReadWriteStream(s, w, t.FieldName);
        SerializeKeys(s, w, t.FloatKeys,   SerializeAnimFloatKey);
        SerializeKeys(s, w, t.Vector3Keys, SerializeAnimVector3Key);
        SerializeKeys(s, w, t.QuatKeys,    SerializeAnimQuatKey);
    }

    // -----------------------------------------------------------------------
    // AnimationClip
    // -----------------------------------------------------------------------
    RCPtr<AnimationClip> AnimationClip::StaticCreate(
        string_view name,
        float duration,
        float fps,
        array_list<SPtr<AnimationTrack>>&& tracks)
    {
        auto self = NewAssetObject<AnimationClip>();
        self->SetIndexName(name);
        self->m_duration = duration;
        self->m_fps      = fps;
        self->m_tracks   = std::move(tracks);
        return self;
    }

    SPtr<BoneAnimationTrack> AnimationClip::FindBoneTrack(const string& boneName) const
    {
        for (auto& t : m_tracks)
        {
            if (t->TrackType == AnimationTrackType::Bone)
            {
                auto boneTrack = sptr_cast<BoneAnimationTrack>(t);
                if (boneTrack && boneTrack->BoneName == boneName)
                    return boneTrack;
            }
        }
        return nullptr;
    }

    void AnimationClip::Serialize(AssetSerializer* s)
    {
        base::Serialize(s);
        auto& stream  = s->GetStream();
        bool  isWrite = s->IsWrite;

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
        {
            int32_t typeInt = 0;
            if (isWrite)
            {
                typeInt = static_cast<int32_t>(t->TrackType);
            }
            sser::ReadWriteStream(stream, isWrite, typeInt);
            if (!isWrite)
            {
                auto trackType = static_cast<AnimationTrackType>(typeInt);
                if (trackType == AnimationTrackType::Bone)
                    t = mksptr(new BoneAnimationTrack());
                else
                    t = mksptr(new PropertyAnimationTrack());
                t->TrackType = trackType;
            }

            sser::ReadWriteStream(stream, isWrite, t->Name);

            if (t->TrackType == AnimationTrackType::Bone)
            {
                auto boneTrack = sptr_cast<BoneAnimationTrack>(t);
                SerializeBoneAnimTrack(stream, isWrite, *boneTrack);
            }
            else
            {
                auto propTrack = sptr_cast<PropertyAnimationTrack>(t);
                SerializePropertyAnimTrack(stream, isWrite, *propTrack);
            }
        }
    }

    void AnimationClip::OnInstantiateAsset(AssetObject* obj)
    {
        base::OnInstantiateAsset(obj);
        auto clip = static_cast<ThisClass*>(obj);
        clip->m_duration = m_duration;
        clip->m_fps      = m_fps;
        clip->m_tracks   = m_tracks;
    }

} // namespace pulsar
