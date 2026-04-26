#pragma once
#include <Pulsar/AssetObject.h>
#include <Pulsar/EngineMath.h>
#include "Skeleton.h"

namespace pulsar
{
    // ---------- 关键帧类型 ----------
    struct AnimVector3Key { float Time; Vector3f Value; };
    struct AnimQuatKey    { float Time; Quat4f   Value; };

    // 单根骨骼的动画轨迹（position / rotation / scale 各自独立采样）
    struct BoneAnimTrack
    {
        string                       BoneName;
        array_list<AnimVector3Key>   PositionKeys;
        array_list<AnimQuatKey>      RotationKeys;
        array_list<AnimVector3Key>   ScaleKeys;
    };

    // AnimationClip：一段动画，引用 Skeleton 做骨骼兼容性校验
    class AnimationClip : public AssetObject
    {
        CORELIB_DEF_TYPE(AssemblyObject_pulsar, pulsar::AnimationClip, AssetObject);
        CORELIB_CLASS_ATTR(new AssetIconAttribute("Editor/Icons/animationclip.png"))
    public:
        using ThisClass = AnimationClip;

        static RCPtr<AnimationClip> StaticCreate(
            string_view                name,
            RCPtr<Skeleton>            skeleton,
            float                      duration,
            float                      fps,
            array_list<BoneAnimTrack>&& tracks);

        // Accessors
        RCPtr<Skeleton>                   GetSkeleton()  const { return m_skeleton; }
        float                             GetDuration()  const { return m_duration; }
        float                             GetFPS()       const { return m_fps; }
        const array_list<BoneAnimTrack>&  GetTracks()    const { return m_tracks; }

        // 按名字查找轨迹，找不到返回 nullptr
        const BoneAnimTrack* FindTrack(const string& boneName) const;

        void Serialize(AssetSerializer* s) override;
        void OnCollectAssetDependencies(array_list<jxcorlib::guid_t>& deps) override;
        void OnInstantiateAsset(AssetObject* obj) override;

    private:
        RCPtr<Skeleton>          m_skeleton;
        float                    m_duration = 0.f; // 秒
        float                    m_fps      = 30.f;
        array_list<BoneAnimTrack> m_tracks;
    };
    DECL_PTR(AnimationClip);

} // namespace pulsar
