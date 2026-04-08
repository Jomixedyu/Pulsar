#include "Components/AnimatorComponent.h"
#include "Components/SkinnedMeshRendererComponent.h"
#include <Pulsar/Node.h>
#include <cmath>

namespace pulsar
{
    AnimatorComponent::AnimatorComponent() = default;

    void AnimatorComponent::BeginComponent()
    {
        base::BeginComponent();
        m_renderer = GetNode()->GetComponent<SkinnedMeshRendererComponent>();

        if (m_playOnStart && m_defaultClip)
            Play(m_defaultClip, m_loop);
    }

    void AnimatorComponent::EndComponent()
    {
        base::EndComponent();
        m_renderer = nullptr;
        m_isPlaying = false;
    }

    void AnimatorComponent::Play(RCPtr<AnimationClip> clip, bool loop)
    {
        m_currentClip = clip;
        m_currentTime = 0.f;
        m_loop        = loop;
        m_isPlaying   = (clip != nullptr);
    }

    void AnimatorComponent::Stop()
    {
        m_isPlaying = false;
    }

    void AnimatorComponent::GetDependenciesAsset(array_list<guid_t>& deps) const
    {
        base::GetDependenciesAsset(deps);
        if (m_defaultClip)
            deps.push_back(m_defaultClip.GetGuid());
    }

    void AnimatorComponent::OnTick(Ticker ticker)
    {
        base::OnTick(ticker);

        if (!m_isPlaying || !m_currentClip)
            return;

        m_currentTime += ticker.deltatime * m_speed;

        const float duration = m_currentClip->GetDuration();
        if (m_currentTime > duration)
        {
            if (m_loop)
                m_currentTime = std::fmod(m_currentTime, duration);
            else
            {
                m_currentTime = duration;
                m_isPlaying   = false;
            }
        }

        SampleAndUpload(m_currentTime);
    }

    void AnimatorComponent::SampleAndUpload(float time)
    {
        if (!m_currentClip || !m_renderer)
            return;

        auto skeleton = m_currentClip->GetSkeleton();
        if (!skeleton) return;

        const auto& bones  = skeleton->GetBones();
        const int   boneCount = (int)bones.size();

        // 每块骨骼的局部变换矩阵（相对父骨骼）
        array_list<Matrix4f> localMatrices(boneCount, Matrix4f(1.f));

        for (int i = 0; i < boneCount; ++i)
        {
            auto* track = m_currentClip->FindTrack(bones[i].Name);
            if (!track)
                continue;

            Vector3f pos   = SampleVector3(track->PositionKeys, time);
            Quat4f   rot   = SampleQuat  (track->RotationKeys,  time);
            Vector3f scale = SampleVector3(track->ScaleKeys,     time);

            // TRS: T=平移矩阵, R=旋转矩阵, S=缩放矩阵
            Matrix4f T = jmath::Translate(pos);
            Matrix4f R = rot.ToMatrix();
            Matrix4f S = jmath::Scale(scale);
            localMatrices[i] = T * R * S;
        }

        // 计算全局变换矩阵（每块骨骼乘上父骨骼的全局矩阵）
        array_list<Matrix4f> globalMatrices(boneCount, Matrix4f(1.f));
        for (int i = 0; i < boneCount; ++i)
        {
            int parent = bones[i].ParentIndex;
            if (parent < 0)
                globalMatrices[i] = localMatrices[i];
            else
                globalMatrices[i] = globalMatrices[parent] * localMatrices[i];
        }

        // 最终骨骼矩阵 = GlobalMatrix * InverseBindMatrix
        array_list<Matrix4f> boneMatrices(boneCount);
        for (int i = 0; i < boneCount; ++i)
            boneMatrices[i] = globalMatrices[i] * bones[i].InverseBindMatrix;

        m_renderer->UpdateBoneMatrices(boneMatrices);
    }

    // -----------------------------------------------------------------------
    // 关键帧采样（线性插值）
    // -----------------------------------------------------------------------
    Vector3f AnimatorComponent::SampleVector3(const array_list<AnimVector3Key>& keys, float time)
    {
        if (keys.empty()) return Vector3f{0, 0, 0};
        if (keys.size() == 1 || time <= keys.front().Time) return keys.front().Value;
        if (time >= keys.back().Time) return keys.back().Value;

        // 二分找插值区间
        int lo = 0, hi = (int)keys.size() - 1;
        while (lo + 1 < hi)
        {
            int mid = (lo + hi) / 2;
            if (keys[mid].Time <= time) lo = mid;
            else hi = mid;
        }

        float t0 = keys[lo].Time, t1 = keys[hi].Time;
        float alpha = (t1 > t0) ? (time - t0) / (t1 - t0) : 0.f;
        // 线性插值 Vector3
        const Vector3f& a = keys[lo].Value;
        const Vector3f& b = keys[hi].Value;
        return Vector3f{a.x + (b.x - a.x) * alpha,
                        a.y + (b.y - a.y) * alpha,
                        a.z + (b.z - a.z) * alpha};
    }

    Quat4f AnimatorComponent::SampleQuat(const array_list<AnimQuatKey>& keys, float time)
    {
        if (keys.empty()) return Quat4f{};
        if (keys.size() == 1 || time <= keys.front().Time) return keys.front().Value;
        if (time >= keys.back().Time) return keys.back().Value;

        int lo = 0, hi = (int)keys.size() - 1;
        while (lo + 1 < hi)
        {
            int mid = (lo + hi) / 2;
            if (keys[mid].Time <= time) lo = mid;
            else hi = mid;
        }

        float t0 = keys[lo].Time, t1 = keys[hi].Time;
        float alpha = (t1 > t0) ? (time - t0) / (t1 - t0) : 0.f;

        // Slerp
        const Quat4f& qa = keys[lo].Value;
        const Quat4f& qb_ref = keys[hi].Value;
        Quat4f qb = qb_ref;

        float dot = Quat4f::Dot(qa, qb);
        // 确保最短路径
        if (dot < 0.f)
        {
            qb.x = -qb.x; qb.y = -qb.y; qb.z = -qb.z; qb.w = -qb.w;
            dot = -dot;
        }

        Quat4f result;
        if (dot > 0.9995f)
        {
            // 角度极小，退化为线性插值再归一化
            result.x = qa.x + (qb.x - qa.x) * alpha;
            result.y = qa.y + (qb.y - qa.y) * alpha;
            result.z = qa.z + (qb.z - qa.z) * alpha;
            result.w = qa.w + (qb.w - qa.w) * alpha;
        }
        else
        {
            float theta0    = std::acos(dot);
            float theta     = theta0 * alpha;
            float sinTheta  = std::sin(theta);
            float sinTheta0 = std::sin(theta0);
            float s0 = std::cos(theta) - dot * sinTheta / sinTheta0;
            float s1 = sinTheta / sinTheta0;
            result.x = s0 * qa.x + s1 * qb.x;
            result.y = s0 * qa.y + s1 * qb.y;
            result.z = s0 * qa.z + s1 * qb.z;
            result.w = s0 * qa.w + s1 * qb.w;
        }
        return Quat4f::NormalizeSafe(result);
    }

} // namespace pulsar
