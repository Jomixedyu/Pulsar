#pragma once
#include <Pulsar/AssetObject.h>
#include <Pulsar/EngineMath.h>

namespace pulsar
{
    // -----------------------------------------------------------------------
    // Keyframe types
    // -----------------------------------------------------------------------
    struct AnimFloatKey    { float Time; float    Value; };
    struct AnimVector3Key  { float Time; Vector3f Value; };
    struct AnimQuatKey     { float Time; Quat4f   Value; };

    // -----------------------------------------------------------------------
    // Track types
    // -----------------------------------------------------------------------
    enum class AnimationTrackType
    {
        Bone,
        Property,
    };

    class AnimationTrack : public Object
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::AnimationTrack, Object);
    public:
        AnimationTrackType TrackType = AnimationTrackType::Bone;
        string Name;
    };
    CORELIB_DECL_SHORTSPTR(AnimationTrack);

    class BoneAnimationTrack : public AnimationTrack
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::BoneAnimationTrack, AnimationTrack);
    public:
        string BoneName;
        array_list<AnimVector3Key> PositionKeys;
        array_list<AnimQuatKey>    RotationKeys;
        array_list<AnimVector3Key> ScaleKeys;
    };
    CORELIB_DECL_SHORTSPTR(BoneAnimationTrack);

    class PropertyAnimationTrack : public AnimationTrack
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::PropertyAnimationTrack, AnimationTrack);
    public:
        string ComponentTypeName;  // e.g. "pulsar::CameraComponent"
        string FieldName;          // e.g. "FOV"

        array_list<AnimFloatKey>   FloatKeys;
        array_list<AnimVector3Key> Vector3Keys;
        array_list<AnimQuatKey>    QuatKeys;
    };
    CORELIB_DECL_SHORTSPTR(PropertyAnimationTrack);

    // -----------------------------------------------------------------------
    // AnimationClip asset
    // -----------------------------------------------------------------------
    class AnimationClip : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::AnimationClip, AssetObject);
        CORELIB_CLASS_ATTR(new AssetIconAttribute("Editor/Icons/animationclip.png"))
    public:
        using ThisClass = AnimationClip;

        static RCPtr<AnimationClip> StaticCreate(
            string_view name,
            float duration,
            float fps,
            array_list<SPtr<AnimationTrack>>&& tracks);

        float GetDuration() const { return m_duration; }
        float GetFPS()      const { return m_fps; }
        const array_list<SPtr<AnimationTrack>>& GetTracks() const { return m_tracks; }

        SPtr<BoneAnimationTrack> FindBoneTrack(const string& boneName) const;

        void Serialize(AssetSerializer* s) override;

    private:
        float m_duration = 0.f;
        float m_fps = 30.f;
        array_list<SPtr<AnimationTrack>> m_tracks;
    };
    DECL_PTR(AnimationClip);

} // namespace pulsar
